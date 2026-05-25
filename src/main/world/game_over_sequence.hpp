#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "ecs/context.hpp"
#include "utils/arena.hpp"
#include "utils/random.hpp"
#include "systems/color/color_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/hidden/hidden_object.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/render_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"

#include "level_manager.hpp"
#include "shrooms_screen.hpp"

namespace game_over_sequence {

struct Config {
  glm::vec4 overlay_color{0.0f, 0.0f, 0.0f, 0.58f};
  float overlay_fade = 0.35f;
  float text_delay = 0.6f;
  float text_fade = 0.4f;
  float shake_duration = 2.2f;
  float total_duration = 3.4f;
  float shake_frequency = 30.0f;
  float shake_amplitude_px = 14.0f;
  float text_font_px = 24.0f;
  glm::vec2 text_position_norm{0.0f, 0.08f};
  int overlay_layer = 110;
  int text_layer = 112;
} config;

inline ecs::Entity* overlay_entity = nullptr;
inline render_system::QuadRenderable* overlay_quad = nullptr;
inline hidden::HiddenObject* overlay_hidden = nullptr;

inline ecs::Entity* text_entity = nullptr;
inline text::TextObject* text_object = nullptr;
inline color::OneColor* text_color = nullptr;
inline hidden::HiddenObject* text_hidden = nullptr;
inline transform::NoRotationTransform* text_transform = nullptr;

inline bool active = false;
inline float elapsed = 0.0f;
inline glm::vec2 last_offset{0.0f, 0.0f};
inline std::vector<ecs::Entity*> shake_targets{};
inline levels::LossInfo active_loss{};
inline float phase_x = 0.0f;
inline float phase_y = 0.0f;

inline float clamp01(float v) {
  return std::min(1.0f, std::max(0.0f, v));
}

inline void set_overlay_alpha(float alpha) {
  if (!overlay_quad) return;
  overlay_quad->color = engine::UIColor{config.overlay_color.x, config.overlay_color.y,
                                        config.overlay_color.z, alpha};
}

inline void set_text_alpha(float alpha) {
  if (!text_color) return;
  text_color->color.w = clamp01(alpha);
}

inline void update_text_layout(const std::string& value) {
  if (!text_object || !text_transform) return;
  text_object->text = value;
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, config.text_font_px);
  const glm::vec2 size{layout.width, layout.height};
  const glm::vec2 center = shrooms::screen::norm_to_pixels(config.text_position_norm);
  text_transform->pos = center - size * 0.5f;
}

inline void gather_shake_targets() {
  shake_targets.clear();
  if (auto* main_scene = scene::get_scene("main")) {
    const auto& entities = main_scene->entities();
    shake_targets.reserve(entities.size());
    for (auto* entity : entities) {
      if (!entity) continue;
      shake_targets.push_back(entity);
    }
  }
}

inline void apply_global_shake(const glm::vec2& offset) {
  const glm::vec2 delta = offset - last_offset;
  if (delta.x == 0.0f && delta.y == 0.0f) return;

  for (auto* entity : shake_targets) {
    if (!entity || entity->is_pending_deletion()) continue;
    auto* transform = entity->get<transform::TransformObject>();
    if (!transform) continue;
    transform->translate(delta);
  }
  last_offset = offset;
}

inline void reset_global_shake() {
  if (last_offset.x == 0.0f && last_offset.y == 0.0f) return;
  for (auto* entity : shake_targets) {
    if (!entity || entity->is_pending_deletion()) continue;
    auto* transform = entity->get<transform::TransformObject>();
    if (!transform) continue;
    transform->translate(glm::vec2{-last_offset.x, -last_offset.y});
  }
  last_offset = glm::vec2{0.0f, 0.0f};
}

inline void begin_sequence(const levels::LossInfo& loss) {
  if (active) return;
  active = true;
  elapsed = 0.0f;
  last_offset = glm::vec2{0.0f, 0.0f};
  active_loss = loss;

  if (overlay_hidden) overlay_hidden->show();
  if (text_hidden) text_hidden->show();
  update_text_layout(levels::loss_reason_label(loss));
  set_overlay_alpha(0.0f);
  set_text_alpha(0.0f);

  gather_shake_targets();

  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->activate();
    main_scene->set_pause(true);
  }
}

inline void finish_sequence() {
  reset_global_shake();
  if (overlay_hidden) overlay_hidden->hide();
  if (text_hidden) text_hidden->hide();
  active = false;
  elapsed = 0.0f;
  shake_targets.clear();
  active_loss = levels::LossInfo{};
  levels::finalize_failure();
}

inline bool is_active() { return active; }

inline glm::vec2 current_shake_offset() {
  return active ? last_offset : glm::vec2{0.0f, 0.0f};
}

struct GameOverSequenceController : public dynamic::DynamicObject {
  GameOverSequenceController() : dynamic::DynamicObject() {}
  ~GameOverSequenceController() override { Component::component_count--; }

  void update() override {
    const float dt = static_cast<float>(ecs::context().delta_seconds);

    if (!active) {
      if (!levels::level_finished && !levels::has_pending_failure()) {
        auto info = levels::evaluate_loss_info();
        if (info.reason != levels::LossReason::None) {
          levels::trigger_failure(info.reason, info.type);
        }
      }

      if (levels::has_pending_failure()) {
        begin_sequence(levels::pending_loss_info());
      }
      return;
    }

    if (!levels::has_pending_failure()) {
      reset_global_shake();
      if (overlay_hidden) overlay_hidden->hide();
      if (text_hidden) text_hidden->hide();
      active = false;
      elapsed = 0.0f;
      shake_targets.clear();
      return;
    }

    elapsed += dt;
    const float overlay_t = config.overlay_fade > 0.0f
                                ? clamp01(elapsed / config.overlay_fade)
                                : 1.0f;
    set_overlay_alpha(config.overlay_color.w * overlay_t);

    const float text_t = config.text_fade > 0.0f
                             ? clamp01((elapsed - config.text_delay) / config.text_fade)
                             : (elapsed >= config.text_delay ? 1.0f : 0.0f);
    set_text_alpha(text_t);

    const float shake_t = config.shake_duration > 0.0f
                              ? clamp01(elapsed / config.shake_duration)
                              : 1.0f;
    const float shake_strength = 1.0f - shake_t;
    const float t = static_cast<float>(ecs::context().time_seconds);
    const float nx = std::sin(t * config.shake_frequency + phase_x);
    const float ny = std::sin(t * config.shake_frequency * 0.92f + phase_y);
    const float amplitude = config.shake_amplitude_px * shake_strength;
    apply_global_shake(glm::vec2{nx * amplitude, ny * amplitude});

    if (elapsed >= config.total_duration) {
      finish_sequence();
    }
  }
};

inline GameOverSequenceController controller{};

inline void init() {
  const glm::vec2 view_size{
      static_cast<float>(shrooms::screen::view_width),
      static_cast<float>(shrooms::screen::view_height),
  };

  overlay_entity = arena::create<ecs::Entity>();
  auto* overlay_transform = arena::create<transform::NoRotationTransform>();
  overlay_transform->pos = glm::vec2{0.0f, 0.0f};
  overlay_entity->add(overlay_transform);
  overlay_entity->add(arena::create<layers::ConstLayer>(config.overlay_layer));
  overlay_quad = arena::create<render_system::QuadRenderable>(
      view_size.x, view_size.y,
      engine::UIColor{config.overlay_color.x, config.overlay_color.y, config.overlay_color.z, 0.0f});
  overlay_entity->add(overlay_quad);
  overlay_hidden = arena::create<hidden::HiddenObject>();
  overlay_hidden->hide();
  overlay_entity->add(overlay_hidden);
  overlay_entity->add(arena::create<scene::SceneObject>("main"));

  text_entity = arena::create<ecs::Entity>();
  text_transform = arena::create<transform::NoRotationTransform>();
  text_entity->add(text_transform);
  text_entity->add(arena::create<layers::ConstLayer>(config.text_layer));
  text_object = arena::create<text::TextObject>("", config.text_font_px);
  text_entity->add(text_object);
  text_color = arena::create<color::OneColor>(glm::vec4{1.0f, 1.0f, 1.0f, 0.0f});
  text_entity->add(text_color);
  text_hidden = arena::create<hidden::HiddenObject>();
  text_hidden->hide();
  text_entity->add(text_hidden);
  text_entity->add(arena::create<scene::SceneObject>("main"));

  const float shake_scale = shrooms::screen::scale_to_pixels(glm::vec2{0.02f, 0.0f}).x;
  config.shake_amplitude_px = std::max(config.shake_amplitude_px, shake_scale);
  phase_x = static_cast<float>(rnd::get_double(0.0, 6.28318));
  phase_y = static_cast<float>(rnd::get_double(0.0, 6.28318));
}

}  // namespace game_over_sequence
