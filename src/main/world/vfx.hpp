#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>

#include "ecs/ecs.hpp"
#include "ecs/context.hpp"
#include "utils/arena.hpp"
#include "utils/random.hpp"
#include "systems/color/color_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/moving/moving_object.hpp"
#include "systems/render/render_system.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/rotating/rotating_object.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"
#include "engine/geometry_builder.h"
#include "engine/resource_ids.h"

namespace vfx {

inline float clamp01(float v) {
  return std::min(1.0f, std::max(0.0f, v));
}

inline float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

inline float ease_out(float t) {
  const float u = 1.0f - t;
  return 1.0f - u * u;
}

inline float ease_in(float t) {
  return t * t;
}

inline void resize_sprite_quad(render_system::SpriteRenderable* sprite, const glm::vec2& size) {
  if (!sprite) return;
  sprite->size = size;
  sprite->geometry = engine::geometry::make_quad(size.x, size.y);
  sprite->uploaded = false;
}

struct BurstConfig {
  std::string texture;
  float lifetime = 0.35f;
  float start_scale = 0.4f;
  float end_scale = 1.2f;
  float start_alpha = 0.9f;
  float end_alpha = 0.0f;
  float base_scale = 1.0f;
  int layer = 4;
  glm::vec4 tint{1.0f, 1.0f, 1.0f, 1.0f};
};

inline BurstConfig spawn_burst{
    "explosion",
    0.25f,
    0.2f,
    1.05f,
    0.75f,
    0.0f,
    0.85f,
    4,
    glm::vec4{0.75f, 0.45f, 1.0f, 1.0f},
};

inline BurstConfig destroy_burst{
    "explosion",
    0.45f,
    0.45f,
    1.4f,
    0.95f,
    0.0f,
    1.05f,
    4,
    glm::vec4{0.45f, 0.95f, 0.7f, 1.0f},
};

inline BurstConfig catch_burst{
    "explosion",
    0.2f,
    0.35f,
    0.95f,
    0.38f,
    0.0f,
    0.75f,
    -1,
    glm::vec4{0.75f, 0.62f, 1.0f, 1.0f},
};

inline BurstConfig sort_burst{
    "explosion",
    0.22f,
    0.65f,
    1.2f,
    0.9f,
    0.0f,
    0.9f,
    5,
    glm::vec4{0.9f, 0.6f, 1.0f, 1.0f},
};

inline BurstConfig projectile_burst{
    "explosion",
    0.2f,
    0.4f,
    1.0f,
    0.8f,
    0.0f,
    0.6f,
    5,
    glm::vec4{0.7f, 0.8f, 1.0f, 1.0f},
};

inline const glm::vec4 floor_lava_color{103.0f / 255.0f, 36.0f / 255.0f,
                                        110.0f / 255.0f, 1.0f};
inline constexpr int kMissBoilMushroomLayer = -2;
inline constexpr int kMissBoilBubbleLayer = -1;

struct VfxBurst : public dynamic::DynamicObject {
  VfxBurst(glm::vec2 center, glm::vec2 base_size, BurstConfig config)
      : dynamic::DynamicObject(),
        center(center),
        base_size(base_size),
        config(std::move(config)) {}
  ~VfxBurst() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    const float t = config.lifetime > 0.0f ? clamp01(elapsed / config.lifetime) : 1.0f;
    const float scale = lerp(config.start_scale, config.end_scale, ease_out(t));
    const float alpha = lerp(config.start_alpha, config.end_alpha, ease_in(t));

    if (auto* sprite = entity->get<render_system::SpriteRenderable>()) {
      sprite->size = base_size * scale;
    }
    if (auto* transform = entity->get<transform::NoRotationTransform>()) {
      const glm::vec2 size = base_size * scale;
      transform->pos = center - size * 0.5f;
    }
    if (auto* tint = entity->get<color::OneColor>()) {
      tint->color.w = alpha;
    }

    if (elapsed >= config.lifetime) {
      entity->mark_deleted();
    }
  }

  glm::vec2 center{0.0f, 0.0f};
  glm::vec2 base_size{0.0f, 0.0f};
  BurstConfig config;
  float elapsed = 0.0f;
};

struct CatchConsumeVanish : public dynamic::DynamicObject {
  CatchConsumeVanish(glm::vec2 start_center, glm::vec2 base_size, glm::vec2 target_center,
                     const transform::NoRotationTransform* x_anchor = nullptr)
      : dynamic::DynamicObject(),
        start_center(start_center),
        base_size(base_size),
        target_center(target_center),
        x_anchor(x_anchor),
        x_anchor_start(x_anchor ? x_anchor->pos.x : 0.0f) {}
  ~CatchConsumeVanish() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;

    auto* sprite = entity->get<render_system::SpriteRenderable>();
    auto* transform = entity->get<transform::NoRotationTransform>();
    if (!sprite || !transform) {
      entity->mark_deleted();
      return;
    }

    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    const float t = duration > 0.0f ? clamp01(elapsed / duration) : 1.0f;

    const float follow_t = ease_out(t);
    const float consume_t = ease_in(t);
    const float consumed_x = std::isfinite(target_center.x) ? target_center.x : start_center.x;
    const float consumed_y = std::isfinite(target_center.y) ? target_center.y : start_center.y;
    const float x_anchor_delta = x_anchor ? x_anchor->pos.x - x_anchor_start : 0.0f;
    const float wobble = std::sin(phase + t * wobble_cycles * 6.28318530718f) * base_size.x *
                         wobble_ratio * (1.0f - follow_t);
    const float y_jitter =
        std::sin(phase * 1.73f + t * 18.0f) * base_size.y * 0.03f * (1.0f - follow_t);
    const glm::vec2 center{
        lerp(start_center.x, consumed_x, follow_t) + x_anchor_delta + wobble,
        lerp(start_center.y, consumed_y, follow_t) + y_jitter,
    };
    const float scale = std::max(min_scale, lerp(1.0f, min_scale, consume_t));
    const glm::vec2 scaled_size = base_size * scale;

    resize_sprite_quad(sprite, scaled_size);
    transform->pos = center - scaled_size * 0.5f;

    if (auto* tint = entity->get<color::OneColor>()) {
      tint->color.w = 1.0f - consume_t;
    }

    if (elapsed >= duration) {
      entity->mark_deleted();
    }
  }

  glm::vec2 start_center{0.0f, 0.0f};
  glm::vec2 base_size{0.0f, 0.0f};
  glm::vec2 target_center{
      std::numeric_limits<float>::quiet_NaN(),
      std::numeric_limits<float>::quiet_NaN(),
  };
  const transform::NoRotationTransform* x_anchor = nullptr;
  float x_anchor_start = 0.0f;
  float duration = 0.32f;
  float phase = static_cast<float>(rnd::get_double(0.0, 6.28318530718));
  float wobble_ratio = 0.03f;
  float wobble_cycles = 1.2f;
  float min_scale = 0.04f;
  float elapsed = 0.0f;
};

struct MissBoilVanish : public dynamic::DynamicObject {
  MissBoilVanish(glm::vec2 start_center, glm::vec2 size, float delete_delay,
                 float sink_distance_px)
      : dynamic::DynamicObject(),
        start_center(start_center),
        size(size),
        delete_delay(delete_delay),
        sink_distance_px(sink_distance_px) {}
  ~MissBoilVanish() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;

    auto* transform = entity->get<transform::NoRotationTransform>();
    if (!transform) {
      entity->mark_deleted();
      return;
    }

    if (auto* moving = entity->get<dynamic::MovingObject>()) {
      moving->translate = glm::vec2{0.0f, 0.0f};
    }
    if (auto* rotating = entity->get<dynamic::RotatingObject>()) {
      rotating->angle = 0.0f;
    }

    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    const float t = delete_delay > 0.0f ? clamp01(elapsed / delete_delay) : 1.0f;
    const glm::vec2 center = start_center + glm::vec2{0.0f, sink_distance_px * ease_in(t)};
    transform->pos = center - size * 0.5f;

    if (elapsed >= delete_delay) {
      entity->mark_deleted();
    }
  }

  glm::vec2 start_center{0.0f, 0.0f};
  glm::vec2 size{0.0f, 0.0f};
  float delete_delay = 0.24f;
  float sink_distance_px = 0.0f;
  float elapsed = 0.0f;
};

struct SporeConfig {
  glm::vec4 color{1.0f, 1.0f, 1.0f, 0.6f};
  float lifetime = 0.45f;
  float start_radius = 2.0f;
  float end_radius = 6.0f;
  glm::vec2 velocity{0.0f, 0.0f};
  int layer = 4;
};

struct ScoreDeltaConfig {
  float lifetime = 0.65f;
  float rise_speed_px = 82.0f;
  float drift_speed_px = 28.0f;
  float wobble_speed = 7.0f;
  float wobble_amplitude_px = 8.0f;
  float font_px = 18.0f;
  glm::vec2 offset_px{0.0f, -12.0f};
  glm::vec4 positive_color{0.7f, 1.0f, 0.75f, 1.0f};
  glm::vec4 negative_color{1.0f, 0.45f, 0.45f, 1.0f};
  int layer = 12;
};

inline ScoreDeltaConfig score_delta_config{};

inline SporeConfig spawn_warning_spore{
    glm::vec4{0.85f, 0.65f, 1.0f, 0.45f},
    0.35f,
    4.0f,
    18.0f,
    glm::vec2{0.0f, 0.0f},
    4,
};

struct VfxSpore : public dynamic::DynamicObject {
  VfxSpore(glm::vec2 center, SporeConfig config)
      : dynamic::DynamicObject(),
        center(center),
        config(std::move(config)) {}
  ~VfxSpore() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    const float t = config.lifetime > 0.0f ? clamp01(elapsed / config.lifetime) : 1.0f;
    const float radius = lerp(config.start_radius, config.end_radius, ease_out(t));
    center += config.velocity * dt;

    if (auto* transform = entity->get<transform::NoRotationTransform>()) {
      transform->pos = center - glm::vec2{radius, radius};
    }
    if (auto* circle = entity->get<render_system::CircleRenderable>()) {
      auto color = config.color;
      color.w = lerp(config.color.w, 0.0f, ease_in(t));
      circle->radius = radius;
      circle->color = engine::UIColor{color.x, color.y, color.z, color.w};
    }

    if (elapsed >= config.lifetime) {
      entity->mark_deleted();
    }
  }

  glm::vec2 center{0.0f, 0.0f};
  SporeConfig config;
  float elapsed = 0.0f;
};

struct BoilBubbleConfig {
  glm::vec2 start_center{0.0f, 0.0f};
  glm::vec2 end_center{0.0f, 0.0f};
  glm::vec4 color{0.72f, 0.28f, 1.0f, 1.0f};
  float start_radius = 3.0f;
  float end_radius = 24.0f;
  float lifetime = 0.7f;
  float delay = 0.0f;
  float start_alpha = 0.0f;
  float peak_alpha = 0.85f;
  float end_alpha = 0.0f;
  float grow_fraction = 0.35f;
  float fade_start = 0.58f;
  float wobble_px = 0.0f;
  float phase = 0.0f;
  int layer = 5;
  int segments = 32;
};

struct BoilBubble : public dynamic::DynamicObject {
  explicit BoilBubble(BoilBubbleConfig config)
      : dynamic::DynamicObject(),
        config(std::move(config)) {}
  ~BoilBubble() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;

    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    const float active_elapsed = elapsed - config.delay;
    if (active_elapsed < 0.0f) {
      apply_state(config.start_center, std::max(0.1f, config.start_radius), 0.0f);
      return;
    }

    const float lifetime = std::max(0.001f, config.lifetime);
    const float t = clamp01(active_elapsed / lifetime);
    const float grow_t = clamp01(t / std::max(0.001f, config.grow_fraction));
    const float fade_t = clamp01((t - config.fade_start) / std::max(0.001f, 1.0f - config.fade_start));
    const float move_t = ease_out(t);
    const float radius = lerp(config.start_radius, config.end_radius, ease_out(grow_t));
    float alpha = lerp(config.start_alpha, config.peak_alpha, ease_out(grow_t));
    if (t >= config.fade_start) {
      alpha = lerp(config.peak_alpha, config.end_alpha, ease_in(fade_t));
    }

    glm::vec2 center{
        lerp(config.start_center.x, config.end_center.x, move_t),
        lerp(config.start_center.y, config.end_center.y, move_t),
    };
    center.x += std::sin(config.phase + active_elapsed * 12.0f) * config.wobble_px *
                (1.0f - clamp01(t));

    apply_state(center, radius, alpha);

    if (active_elapsed >= config.lifetime) {
      entity->mark_deleted();
    }
  }

  void apply_state(const glm::vec2& center, float radius, float alpha) {
    radius = std::max(0.1f, radius);
    if (auto* transform = entity->get<transform::NoRotationTransform>()) {
      transform->pos = center - glm::vec2{radius, radius};
    }
    if (auto* circle = entity->get<render_system::CircleRenderable>()) {
      if (std::fabs(circle->radius - radius) > 0.1f) {
        circle->radius = radius;
        circle->geometry = engine::geometry::make_circle(radius, config.segments);
        circle->uploaded = false;
      }
      glm::vec4 color = config.color;
      color.w = alpha;
      circle->color = engine::UIColor{color.x, color.y, color.z, color.w};
    }
  }

  BoilBubbleConfig config;
  float elapsed = 0.0f;
};

struct SpriteShatterPiece : public dynamic::DynamicObject {
  SpriteShatterPiece(glm::vec2 center, glm::vec2 size, glm::vec2 velocity, float gravity,
                     float lifetime)
      : dynamic::DynamicObject(),
        center(center),
        size(size),
        velocity(velocity),
        gravity(gravity),
        lifetime(lifetime) {}
  ~SpriteShatterPiece() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    velocity.y += gravity * dt;
    center += velocity * dt;
    const float t = lifetime > 0.0f ? clamp01(elapsed / lifetime) : 1.0f;

    if (auto* transform = entity->get<transform::NoRotationTransform>()) {
      transform->pos = center - size * 0.5f;
    }
    if (auto* tint = entity->get<color::OneColor>()) {
      tint->color.w = 1.0f - ease_in(t);
    }

    if (elapsed >= lifetime) {
      entity->mark_deleted();
    }
  }

  glm::vec2 center{0.0f, 0.0f};
  glm::vec2 size{0.0f, 0.0f};
  glm::vec2 velocity{0.0f, 0.0f};
  float gravity = 0.0f;
  float lifetime = 0.3f;
  float elapsed = 0.0f;
};

struct ScoreDeltaText : public dynamic::DynamicObject {
  ScoreDeltaText(glm::vec2 center, glm::vec2 size, glm::vec4 color, float lifetime,
                 float rise_speed_px, float drift_speed_px, float wobble_speed,
                 float wobble_amplitude_px)
      : dynamic::DynamicObject(),
        center(center),
        size(size),
        base_color(color),
        lifetime(lifetime),
        rise_speed_px(rise_speed_px),
        drift_speed_px(drift_speed_px),
        wobble_speed(wobble_speed),
        wobble_amplitude_px(wobble_amplitude_px),
        drift_dir(static_cast<float>(rnd::get_double(-1.0, 1.0))),
        wobble_phase(static_cast<float>(rnd::get_double(0.0, 6.28318530718))) {}
  ~ScoreDeltaText() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;
    const float t = lifetime > 0.0f ? clamp01(elapsed / lifetime) : 1.0f;
    center.y -= rise_speed_px * dt;
    center.x += drift_dir * drift_speed_px * dt;
    const float wobble =
        std::sin(wobble_phase + elapsed * wobble_speed * 6.28318530718f) * wobble_amplitude_px;

    if (auto* transform = entity->get<transform::NoRotationTransform>()) {
      transform->pos = glm::vec2{center.x + wobble, center.y} - size * 0.5f;
    }
    if (auto* tint = entity->get<color::OneColor>()) {
      tint->color = base_color;
      tint->color.w = lerp(base_color.w, 0.0f, ease_in(t));
    }

    if (elapsed >= lifetime) {
      entity->mark_deleted();
    }
  }

  glm::vec2 center{0.0f, 0.0f};
  glm::vec2 size{0.0f, 0.0f};
  glm::vec4 base_color{1.0f, 1.0f, 1.0f, 1.0f};
  float lifetime = 0.0f;
  float rise_speed_px = 0.0f;
  float drift_speed_px = 0.0f;
  float wobble_speed = 0.0f;
  float wobble_amplitude_px = 0.0f;
  float drift_dir = 0.0f;
  float wobble_phase = 0.0f;
  float elapsed = 0.0f;
};

struct WobbleOffset;
COMPONENT_VECTOR(WobbleOffset, wobble_offsets);

struct WobbleOffset : public dynamic::DynamicObject {
  WobbleOffset(glm::vec2 amplitude, float speed, float phase, bool respect_pause)
      : dynamic::DynamicObject(),
        amplitude(amplitude),
        speed(speed),
        phase(phase),
        respect_pause(respect_pause) {
    wobble_offsets.push_back(this);
  }
  ~WobbleOffset() override { Component::component_count--; }

  void update() override {
    if (respect_pause && scene::is_current_scene_paused()) return;
    if (!entity || entity->is_pending_deletion()) return;
    auto* transform = entity->get<transform::TransformObject>();
    if (!transform) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    anim_time += std::max(0.0f, dt);
    const float t = anim_time;
    const glm::vec2 offset{
        std::sin(t * speed + phase) * amplitude.x,
        std::sin(t * speed * 0.73f + phase * 1.4f) * amplitude.y,
    };
    transform->translate(offset - last_offset);
    last_offset = offset;
  }

  glm::vec2 amplitude{0.0f, 0.0f};
  float speed = 1.0f;
  float phase = 0.0f;
  bool respect_pause = true;
  float anim_time = 0.0f;
  glm::vec2 last_offset{0.0f, 0.0f};
  DETACH_VECTOR(WobbleOffset, wobble_offsets)
};

inline void reset_wobble_offsets() {
  for (auto* wobble : wobble_offsets) {
    if (!wobble) continue;
    auto* entity = wobble->get_entity();
    if (!entity || entity->is_pending_deletion()) continue;
    auto* transform = entity->get<transform::TransformObject>();
    if (!transform) continue;
    if (wobble->last_offset.x != 0.0f || wobble->last_offset.y != 0.0f) {
      transform->translate(glm::vec2{-wobble->last_offset.x, -wobble->last_offset.y});
    }
    wobble->last_offset = glm::vec2{0.0f, 0.0f};
    wobble->anim_time = 0.0f;
    wobble->phase = 0.0f;
  }
}

inline void attach_wobble(ecs::Entity* entity, const glm::vec2& amplitude_px, float speed,
                          bool respect_pause = true) {
  if (!entity) return;
  const float phase = static_cast<float>(rnd::get_double(0.0, 6.28318));
  entity->add(arena::create<WobbleOffset>(amplitude_px, speed, phase, respect_pause));
}

inline void spawn_spore(const glm::vec2& center, const SporeConfig& config) {
  auto* entity = arena::create<ecs::Entity>();
  const float radius = config.start_radius;
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = center - glm::vec2{radius, radius};
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(config.layer));
  entity->add(arena::create<render_system::CircleRenderable>(
      radius, engine::UIColor{config.color.x, config.color.y, config.color.z, config.color.w}));
  entity->add(arena::create<VfxSpore>(center, config));
  entity->add(arena::create<scene::SceneObject>("main"));
}

inline void spawn_boil_bubble(const BoilBubbleConfig& config) {
  auto* entity = arena::create<ecs::Entity>();
  const float radius = std::max(0.1f, config.start_radius);
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = config.start_center - glm::vec2{radius, radius};
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(config.layer));
  glm::vec4 color = config.color;
  color.w = config.start_alpha;
  entity->add(arena::create<render_system::CircleRenderable>(
      radius, engine::UIColor{color.x, color.y, color.z, color.w}));
  entity->add(arena::create<BoilBubble>(config));
  entity->add(arena::create<scene::SceneObject>("main"));
}

inline void spawn_spore_cloud(const glm::vec2& center, float base_radius, int count,
                              const glm::vec4& color, float spread_px, float speed_px,
                              float lifetime, int layer) {
  for (int i = 0; i < count; ++i) {
    const float ang = static_cast<float>(rnd::get_double(0.0, 6.28318));
    const float dist = static_cast<float>(rnd::get_double(0.0, spread_px));
    const glm::vec2 offset{std::cos(ang) * dist, std::sin(ang) * dist};
    const float vel = static_cast<float>(rnd::get_double(speed_px * 0.4f, speed_px));
    const glm::vec2 velocity{std::cos(ang) * vel, std::sin(ang) * vel - speed_px * 0.3f};
    SporeConfig config{};
    config.color = color;
    config.lifetime = static_cast<float>(rnd::get_double(lifetime * 0.7f, lifetime * 1.15f));
    config.start_radius = static_cast<float>(rnd::get_double(base_radius * 0.6f, base_radius));
    config.end_radius = static_cast<float>(rnd::get_double(base_radius * 1.2f, base_radius * 2.2f));
    config.velocity = velocity;
    config.layer = layer;
    spawn_spore(center + offset, config);
  }
}

inline void spawn_burst_at(const glm::vec2& center, const glm::vec2& size,
                           const BurstConfig& config) {
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  const glm::vec2 scaled_size = size * config.base_scale;
  transform->pos = center - scaled_size * 0.5f;
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(config.layer));
  const engine::TextureId tex_id = engine::resources::register_texture(config.texture);
  entity->add(arena::create<render_system::SpriteRenderable>(tex_id, scaled_size));
  glm::vec4 tint = config.tint;
  tint.w = config.start_alpha;
  entity->add(arena::create<color::OneColor>(tint));
  entity->add(arena::create<VfxBurst>(center, scaled_size, config));
  entity->add(arena::create<scene::SceneObject>("main"));
}

inline glm::vec2 entity_size(ecs::Entity* entity) {
  if (!entity) return glm::vec2{0.0f, 0.0f};
  if (auto* sprite = entity->get<render_system::SpriteRenderable>()) {
    return sprite->size;
  }
  return glm::vec2{0.0f, 0.0f};
}

inline glm::vec2 entity_center(ecs::Entity* entity, const glm::vec2& size) {
  if (!entity) return glm::vec2{0.0f, 0.0f};
  if (auto* transform = entity->get<transform::NoRotationTransform>()) {
    return transform->pos + size * 0.5f;
  }
  return glm::vec2{0.0f, 0.0f};
}

inline bool is_catch_animating(const ecs::Entity* entity) {
  return entity && entity->get<CatchConsumeVanish>();
}

inline bool is_mushroom_vfx_locked(const ecs::Entity* entity) {
  return entity && (entity->get<CatchConsumeVanish>() || entity->get<MissBoilVanish>());
}

inline void start_catch_consume_vanish(
    ecs::Entity* entity,
    glm::vec2 target_center = glm::vec2{std::numeric_limits<float>::quiet_NaN(),
                                        std::numeric_limits<float>::quiet_NaN()},
    const transform::NoRotationTransform* x_anchor = nullptr) {
  if (!entity || entity->is_pending_deletion()) return;
  if (is_mushroom_vfx_locked(entity)) return;

  auto* sprite = entity->get<render_system::SpriteRenderable>();
  auto* transform = entity->get<transform::NoRotationTransform>();
  if (!sprite || !transform) {
    entity->mark_deleted();
    return;
  }

  const glm::vec2 size = entity_size(entity);
  if (size.x <= 0.0f || size.y <= 0.0f) {
    entity->mark_deleted();
    return;
  }

  if (auto* moving = entity->get<dynamic::MovingObject>()) {
    moving->translate = glm::vec2{0.0f, 0.0f};
  }
  if (auto* rotating = entity->get<dynamic::RotatingObject>()) {
    rotating->angle = 0.0f;
  }
  if (!entity->get<color::OneColor>()) {
    entity->add(arena::create<color::OneColor>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}));
  }
  constexpr int kConsumeLayer = -1;
  if (auto* layer = entity->get<layers::ConstLayer>()) {
    layer->layer = std::min(layer->layer, kConsumeLayer);
  } else {
    entity->add(arena::create<layers::ConstLayer>(kConsumeLayer));
  }

  const glm::vec2 center = entity_center(entity, size);
  entity->add(arena::create<CatchConsumeVanish>(center, size, target_center, x_anchor));
}

inline void spawn_spawn_effect(const glm::vec2& center, const glm::vec2& size) {
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  const float extent = std::max(size.x, size.y);
  const float base_radius = std::max(2.0f, extent * 0.06f);
  const float spread = extent * 0.35f;
  const float speed = std::max(30.0f, extent * 0.9f);
  spawn_spore_cloud(center, base_radius, 6, glm::vec4{0.55f, 0.9f, 0.75f, 0.55f}, spread,
                    speed, 0.5f, spawn_burst.layer + 1);
}

inline void spawn_catch_effect(
    ecs::Entity* entity,
    glm::vec2 target_center = glm::vec2{std::numeric_limits<float>::quiet_NaN(),
                                        std::numeric_limits<float>::quiet_NaN()},
    const transform::NoRotationTransform* x_anchor = nullptr) {
  if (!entity) return;
  if (is_mushroom_vfx_locked(entity)) return;
  const glm::vec2 size = entity_size(entity);
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  const glm::vec2 center = entity_center(entity, size);
  start_catch_consume_vanish(entity, target_center, x_anchor);
  spawn_burst_at(center, size * 0.85f, catch_burst);
  const float extent = std::max(size.x, size.y);
  const float base_radius = std::max(1.8f, extent * 0.045f);
  const float spread = extent * 0.25f;
  const float speed = std::max(24.0f, extent * 0.72f);
  spawn_spore_cloud(center, base_radius, 4, glm::vec4{0.78f, 0.68f, 1.0f, 0.42f}, spread, speed,
                    0.32f, catch_burst.layer);
}

inline bool spawn_miss_effect(ecs::Entity* entity) {
  if (!entity || entity->is_pending_deletion()) return false;
  if (is_mushroom_vfx_locked(entity)) return false;
  const glm::vec2 size = entity_size(entity);
  if (size.x <= 0.0f || size.y <= 0.0f) return false;
  const glm::vec2 center = entity_center(entity, size);
  const float extent = std::max(size.x, size.y);
  const float delete_delay = 0.24f;
  const float sink_distance = std::max(6.0f, extent * 0.22f);
  if (auto* layer = entity->get<layers::ConstLayer>()) {
    layer->layer = std::min(layer->layer, kMissBoilMushroomLayer);
  } else {
    entity->add(arena::create<layers::ConstLayer>(kMissBoilMushroomLayer));
  }
  entity->add(arena::create<MissBoilVanish>(center, size, delete_delay, sink_distance));

  const glm::vec2 lava_center = center + glm::vec2{0.0f, size.y * 0.46f};
  const glm::vec2 gulp_center = center + glm::vec2{0.0f, size.y * 0.08f};
  const float cluster_spread_x = std::max(5.0f, extent * 0.24f);
  const float cluster_spread_y = std::max(4.0f, extent * 0.14f);

  const int gulp_count = rnd::get_int(7, 10);
  for (int i = 0; i < gulp_count; ++i) {
    const float angle = static_cast<float>(rnd::get_double(0.0, 6.28318530718));
    const float radius_t = std::sqrt(static_cast<float>(rnd::get_double(0.0, 1.0)));
    const glm::vec2 cluster_offset{
        std::cos(angle) * cluster_spread_x * radius_t,
        std::sin(angle) * cluster_spread_y * radius_t,
    };
    const glm::vec2 rise_jitter{
        static_cast<float>(rnd::get_double(-extent * 0.04f, extent * 0.04f)),
        static_cast<float>(rnd::get_double(extent * 0.02f, extent * 0.12f)),
    };
    const float start_radius =
        static_cast<float>(rnd::get_double(std::max(1.5f, extent * 0.045f),
                                           std::max(1.5f, extent * 0.045f)));
    const float end_radius =
        static_cast<float>(rnd::get_double(std::max(start_radius * 1.8f, extent * 0.075f),
                                           std::max(start_radius * 2.6f, extent * 0.13f)));
    BoilBubbleConfig gulp{};
    gulp.start_center = lava_center + cluster_offset * 0.45f + rise_jitter;
    gulp.end_center = gulp_center + cluster_offset;
    gulp.color = floor_lava_color;
    gulp.start_radius = start_radius;
    gulp.end_radius = end_radius;
    gulp.lifetime = static_cast<float>(rnd::get_double(0.66, 0.78));
    gulp.delay = static_cast<float>(rnd::get_double(0.0, 0.12)) + static_cast<float>(i) * 0.008f;
    gulp.start_alpha = 1.0f;
    gulp.peak_alpha = 1.0f;
    gulp.end_alpha = 0.0f;
    gulp.grow_fraction = 0.34f;
    gulp.fade_start = 0.58f;
    gulp.wobble_px = extent * 0.018f;
    gulp.phase = static_cast<float>(rnd::get_double(0.0, 6.28318530718));
    gulp.layer = kMissBoilBubbleLayer;
    gulp.segments = 36;
    spawn_boil_bubble(gulp);
  }

  const int small_count = 9;
  const float spread_x = std::max(8.0f, size.x * 0.7f);
  for (int i = 0; i < small_count; ++i) {
    const float delay = static_cast<float>(rnd::get_double(0.0, 0.16));
    const float side = static_cast<float>(rnd::get_double(-1.0, 1.0));
    const float start_radius = static_cast<float>(
        rnd::get_double(std::max(2.0f, extent * 0.045f), std::max(3.5f, extent * 0.12f)));
    BoilBubbleConfig bubble{};
    bubble.start_center =
        lava_center + glm::vec2{side * spread_x * static_cast<float>(rnd::get_double(0.05, 0.55)),
                                static_cast<float>(rnd::get_double(0.0, extent * 0.2f))};
    bubble.end_center =
        bubble.start_center + glm::vec2{side * static_cast<float>(rnd::get_double(2.0, 12.0)),
                                        -static_cast<float>(rnd::get_double(extent * 0.45f,
                                                                            extent * 0.95f))};
    bubble.color = floor_lava_color;
    bubble.start_radius = start_radius;
    bubble.end_radius = start_radius * static_cast<float>(rnd::get_double(1.7, 2.8));
    bubble.lifetime = static_cast<float>(rnd::get_double(0.45, 0.82));
    bubble.delay = delay;
    bubble.start_alpha = 1.0f;
    bubble.peak_alpha = 1.0f;
    bubble.end_alpha = 0.0f;
    bubble.grow_fraction = static_cast<float>(rnd::get_double(0.26, 0.42));
    bubble.fade_start = static_cast<float>(rnd::get_double(0.42, 0.58));
    bubble.wobble_px = static_cast<float>(rnd::get_double(extent * 0.03f, extent * 0.14f));
    bubble.phase = static_cast<float>(rnd::get_double(0.0, 6.28318530718));
    bubble.layer = kMissBoilBubbleLayer;
    bubble.segments = 28;
    spawn_boil_bubble(bubble);
  }

  return true;
}

inline void spawn_sort_effect(ecs::Entity* entity) {
  if (!entity) return;
  const glm::vec2 size = entity_size(entity);
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  const glm::vec2 center = entity_center(entity, size);
  spawn_burst_at(center, size, sort_burst);
  const float extent = std::max(size.x, size.y);
  const float base_radius = std::max(2.0f, extent * 0.05f);
  const float spread = extent * 0.25f;
  const float speed = std::max(35.0f, extent * 0.9f);
  spawn_spore_cloud(center, base_radius, 7, glm::vec4{0.85f, 0.65f, 1.0f, 0.6f}, spread, speed,
                    0.4f, sort_burst.layer + 1);
}

inline void spawn_sprite_shatter(ecs::Entity* entity, int columns = 3, int rows = 3) {
  if (!entity) return;
  auto* sprite = entity->get<render_system::SpriteRenderable>();
  auto* transform = entity->get<transform::NoRotationTransform>();
  if (!sprite || !transform) return;
  if (sprite->size.x <= 0.0f || sprite->size.y <= 0.0f) return;

  columns = std::max(1, columns);
  rows = std::max(1, rows);
  const glm::vec2 piece_size{
      sprite->size.x / static_cast<float>(columns),
      sprite->size.y / static_cast<float>(rows),
  };
  if (piece_size.x <= 0.0f || piece_size.y <= 0.0f) return;

  int base_layer = sort_burst.layer + 1;
  if (auto* layered = entity->get<layers::LayeredObject>()) {
    base_layer = layered->get_layer() + 1;
  }

  const glm::vec2 center = transform->pos + sprite->size * 0.5f;
  const float extent = std::max(sprite->size.x, sprite->size.y);
  const float min_speed = std::max(90.0f, extent * 1.6f);
  const float max_speed = std::max(130.0f, extent * 2.35f);

  for (int y = 0; y < rows; ++y) {
    for (int x = 0; x < columns; ++x) {
      auto* piece = arena::create<ecs::Entity>();

      const glm::vec2 top_left = transform->pos +
                                 glm::vec2{piece_size.x * static_cast<float>(x),
                                           piece_size.y * static_cast<float>(y)};
      const glm::vec2 piece_center = top_left + piece_size * 0.5f;

      auto* piece_transform = arena::create<transform::NoRotationTransform>();
      piece_transform->pos = top_left;
      piece->add(piece_transform);
      piece->add(arena::create<layers::ConstLayer>(base_layer));

      auto* piece_sprite =
          arena::create<render_system::SpriteRenderable>(sprite->texture_id, piece_size);
      const float u0 = static_cast<float>(x) / static_cast<float>(columns);
      const float v0 = static_cast<float>(y) / static_cast<float>(rows);
      const float u1 = static_cast<float>(x + 1) / static_cast<float>(columns);
      const float v1 = static_cast<float>(y + 1) / static_cast<float>(rows);
      const std::array<float, 8> uvs{u0, v0, u1, v0, u0, v1, u1, v1};
      piece_sprite->geometry = engine::geometry::make_quad(piece_size.x, piece_size.y, uvs);
      piece->add(piece_sprite);
      piece->add(arena::create<color::OneColor>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}));

      glm::vec2 direction = piece_center - center;
      if (glm::length(direction) <= 0.0001f) {
        direction = glm::vec2{
            static_cast<float>(rnd::get_double(-0.8, 0.8)),
            static_cast<float>(rnd::get_double(-1.0, 0.4)),
        };
      }
      direction.x += static_cast<float>(rnd::get_double(-0.28, 0.28));
      direction.y += static_cast<float>(rnd::get_double(-0.24, 0.24));
      if (glm::length(direction) <= 0.0001f) {
        direction = glm::vec2{0.0f, -1.0f};
      } else {
        direction = glm::normalize(direction);
      }

      const float speed = static_cast<float>(rnd::get_double(min_speed, max_speed));
      const glm::vec2 velocity = direction * speed + glm::vec2{0.0f, -speed * 0.18f};
      const float lifetime = static_cast<float>(rnd::get_double(0.24, 0.38));
      piece->add(arena::create<SpriteShatterPiece>(piece_center, piece_size, velocity, 540.0f,
                                                   lifetime));
      piece->add(arena::create<scene::SceneObject>("main"));
    }
  }
}

inline void spawn_projectile_flash(const glm::vec2& center, const glm::vec2& size) {
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  spawn_burst_at(center, size, projectile_burst);
}

inline void spawn_projectile_trail(const glm::vec2& center, float radius) {
  SporeConfig config{};
  config.color = glm::vec4{0.7f, 0.6f, 1.0f, 0.45f};
  config.lifetime = 0.18f;
  config.start_radius = std::max(1.5f, radius * 0.45f);
  config.end_radius = std::max(2.5f, radius * 0.85f);
  config.velocity = glm::vec2{0.0f, 20.0f};
  config.layer = projectile_burst.layer + 1;
  spawn_spore(center, config);
}

inline void spawn_spawn_warning(const glm::vec2& center, const glm::vec2& size, float lifetime) {
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  SporeConfig config = spawn_warning_spore;
  config.lifetime = std::max(0.12f, lifetime);
  const float extent = std::max(size.x, size.y);
  config.start_radius = std::max(2.0f, extent * 0.18f);
  config.end_radius = std::max(config.start_radius + 2.0f, extent * 0.6f);
  spawn_spore(center, config);
}

inline void spawn_destroy_effect(ecs::Entity* entity) {
  if (!entity) return;
  spawn_sort_effect(entity);
  spawn_sprite_shatter(entity, 3, 3);
  const glm::vec2 size = entity_size(entity);
  if (size.x <= 0.0f || size.y <= 0.0f) return;
  const glm::vec2 center = entity_center(entity, size);
  const float extent = std::max(size.x, size.y);
  const float base_radius = std::max(2.0f, extent * 0.045f);
  const float spread = extent * 0.28f;
  const float speed = std::max(25.0f, extent * 0.82f);
  spawn_spore_cloud(center, base_radius, 6, glm::vec4{0.82f, 0.76f, 0.95f, 0.6f}, spread, speed,
                    0.34f, sort_burst.layer + 2);
}

inline void spawn_score_delta(const glm::vec2& center, int delta) {
  if (delta == 0) return;
  const std::string text = (delta > 0 ? "+" : "") + std::to_string(delta);
  const auto layout = engine::text::layout_text(text, 0.0f, 0.0f, score_delta_config.font_px);
  const glm::vec2 size{layout.width, layout.height};
  const glm::vec2 start = center + score_delta_config.offset_px;
  const glm::vec4 color = delta > 0 ? score_delta_config.positive_color : score_delta_config.negative_color;

  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = start - size * 0.5f;
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(score_delta_config.layer));
  entity->add(arena::create<text::TextObject>(text, score_delta_config.font_px));
  entity->add(arena::create<color::OneColor>(color));
  entity->add(arena::create<ScoreDeltaText>(
      start, size, color, score_delta_config.lifetime, score_delta_config.rise_speed_px,
      score_delta_config.drift_speed_px, score_delta_config.wobble_speed,
      score_delta_config.wobble_amplitude_px));
  entity->add(arena::create<scene::SceneObject>("main"));
}

}  // namespace vfx
