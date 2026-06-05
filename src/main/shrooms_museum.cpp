#include "shrooms_museum.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ecs/context.hpp"
#include "ecs/ecs.hpp"
#include "engine/geometry_builder.h"
#include "engine/resource_ids.h"
#include "engine/text_render.h"
#include "engine/ui_stream.h"
#include "systems/animation/animation_system.hpp"
#include "systems/animation/sprite_animation.hpp"
#include "systems/color/color_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/render_system.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"
#include "utils/arena.hpp"
#include "world/player.hpp"
#include "world/shrooms_assets.hpp"
#include "world/shrooms_screen.hpp"
#include "world/shrooms_texture_sizing.hpp"
#include "world/vfx.hpp"

namespace engine::shrooms {

namespace {

constexpr const char* kSceneName = "main";
constexpr float kTabSeconds = 10.0f;
constexpr float kTopChromePx = 86.0f;

constexpr std::array<std::string_view, 7> kPageNames{
    "backgrounds",
    "sprites",
    "player animations",
    "mushroom animations",
    "bat animations",
    "hud samples",
    "font",
};

enum class FamiliarMuseumMode {
  Shoot,
  Trap,
  Return,
};

engine::UIColor ui_color(float r, float g, float b, float a = 1.0f) {
  return engine::UIColor{r, g, b, a};
}

glm::vec4 glm_color(float r, float g, float b, float a = 1.0f) {
  return glm::vec4{r, g, b, a};
}

float clamp01(float value) {
  return std::clamp(value, 0.0f, 1.0f);
}

float smooth01(float value) {
  const float t = clamp01(value);
  return t * t * (3.0f - 2.0f * t);
}

glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, float t) {
  return a + (b - a) * clamp01(t);
}

float page_time(double time_seconds) {
  const float t = static_cast<float>(std::fmod(std::max(0.0, time_seconds),
                                               static_cast<double>(kTabSeconds)));
  return t < 0.0f ? t + kTabSeconds : t;
}

int page_index_for_time(double time_seconds) {
  const double safe_time = std::max(0.0, time_seconds);
  return static_cast<int>(std::floor(safe_time / kTabSeconds)) %
         static_cast<int>(kPageNames.size());
}

glm::vec2 top_left_for_center(const glm::vec2& center, const glm::vec2& size) {
  return center - size * 0.5f;
}

glm::vec2 fit_texture_size(std::string_view texture_name, float max_w, float max_h) {
  const float aspect =
      std::max(0.01f, ::shrooms::texture_sizing::aspect_ratio(texture_name));
  float w = std::max(1.0f, max_w);
  float h = w / aspect;
  if (h > max_h) {
    h = std::max(1.0f, max_h);
    w = h * aspect;
  }
  return glm::vec2{w, h};
}

glm::vec2 mushroom_display_size(std::string_view texture_name) {
  return ::shrooms::texture_sizing::from_reference_width(texture_name, 28.0f);
}

ecs::Entity* create_rect(const glm::vec2& top_left, const glm::vec2& size,
                         const glm::vec4& color, int layer) {
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = top_left;
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(layer));
  entity->add(arena::create<render_system::QuadRenderable>(
      size.x, size.y, ui_color(color.x, color.y, color.z, color.w)));
  entity->add(arena::create<scene::SceneObject>(kSceneName));
  return entity;
}

ecs::Entity* create_text(const std::string& value, const glm::vec2& top_left,
                         float font_px, const glm::vec4& color, int layer) {
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = top_left;
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(layer));
  entity->add(arena::create<::text::TextObject>(value, font_px));
  entity->add(arena::create<color::OneColor>(color));
  entity->add(arena::create<scene::SceneObject>(kSceneName));
  return entity;
}

ecs::Entity* create_centered_text(const std::string& value, const glm::vec2& center,
                                  float font_px, const glm::vec4& color, int layer) {
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, font_px);
  return create_text(value, glm::vec2{center.x - layout.width * 0.5f,
                                      center.y - layout.height * 0.5f},
                     font_px, color, layer);
}

ecs::Entity* create_sprite(engine::TextureId texture_id, const glm::vec2& center,
                           const glm::vec2& size, int layer,
                           const glm::vec4& tint = glm::vec4{1.0f}) {
  if (texture_id == engine::kInvalidTextureId || size.x <= 0.0f || size.y <= 0.0f) {
    return nullptr;
  }
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = top_left_for_center(center, size);
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(layer));
  entity->add(arena::create<render_system::SpriteRenderable>(texture_id, size));
  entity->add(arena::create<color::OneColor>(tint));
  entity->add(arena::create<scene::SceneObject>(kSceneName));
  return entity;
}

ecs::Entity* create_sprite(std::string_view texture_name, const glm::vec2& center,
                           const glm::vec2& size, int layer,
                           const glm::vec4& tint = glm::vec4{1.0f}) {
  return create_sprite(engine::resources::register_texture(std::string{texture_name}),
                       center, size, layer, tint);
}

void create_caption(std::string_view label, const glm::vec2& center, float y, int layer = 20) {
  const float font_px = 14.0f;
  const std::string value{label};
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, font_px);
  create_text(value, glm::vec2{center.x - layout.width * 0.5f, y}, font_px,
              glm_color(0.82f, 0.86f, 0.92f, 0.95f), layer);
}

void add_face_animation(ecs::Entity* entity) {
  if (!entity) return;
  const engine::TextureId frame_1 = engine::resources::register_texture("face_mini_1");
  const engine::TextureId frame_2 = engine::resources::register_texture("face_mini_2");
  std::map<std::string, std::vector<animation::SpriteFrame>> clips{};
  clips["idle"] = {animation::SpriteFrame{frame_1, 0.25f},
                   animation::SpriteFrame{frame_2, 0.25f}};
  entity->add(arena::create<animation::SpriteAnimation>(std::move(clips), "idle"));
}

void clear_scene_entities() {
  auto* main_scene = scene::get_scene(kSceneName);
  if (!main_scene) return;
  const auto entities = main_scene->entities();
  for (auto* entity : entities) {
    if (entity) entity->mark_deleted();
  }
}

template <typename Array>
void append_names(std::vector<std::string_view>& out, const Array& names) {
  for (std::string_view name : names) {
    out.push_back(name);
  }
}

struct LoopingSlide : public dynamic::DynamicObject {
  LoopingSlide(transform::NoRotationTransform* transform, glm::vec2 start, glm::vec2 end,
               float period)
      : dynamic::DynamicObject(), transform(transform), start(start), end(end), period(period) {}

  ~LoopingSlide() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion() || !transform) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += std::max(0.0f, dt);
    const float safe_period = std::max(0.1f, period);
    float p = std::fmod(elapsed, safe_period) / safe_period;
    if (p < 0.0f) p += 1.0f;
    const float forward = p < 0.5f ? p * 2.0f : (1.0f - p) * 2.0f;
    transform->pos = lerp(start, end, smooth01(forward));
  }

  transform::NoRotationTransform* transform = nullptr;
  glm::vec2 start{0.0f, 0.0f};
  glm::vec2 end{0.0f, 0.0f};
  float period = 2.0f;
  float elapsed = 0.0f;
};

struct LoopingMushroomFall : public dynamic::DynamicObject {
  LoopingMushroomFall(transform::NoRotationTransform* transform, color::OneColor* tint,
                      glm::vec2 start_center, glm::vec2 size, float fall_px, float period)
      : dynamic::DynamicObject(),
        transform(transform),
        tint(tint),
        start_center(start_center),
        size(size),
        fall_px(fall_px),
        period(period) {}

  ~LoopingMushroomFall() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion() || !transform) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += std::max(0.0f, dt);
    const float safe_period = std::max(0.1f, period);
    float p = std::fmod(elapsed, safe_period) / safe_period;
    if (p < 0.0f) p += 1.0f;
    const glm::vec2 center = start_center + glm::vec2{0.0f, fall_px * p};
    transform->pos = top_left_for_center(center, size);
    if (tint) {
      const float fade = p > 0.82f ? 1.0f - clamp01((p - 0.82f) / 0.18f) : 1.0f;
      tint->color.w = fade;
    }
  }

  transform::NoRotationTransform* transform = nullptr;
  color::OneColor* tint = nullptr;
  glm::vec2 start_center{0.0f, 0.0f};
  glm::vec2 size{0.0f, 0.0f};
  float fall_px = 220.0f;
  float period = 2.6f;
  float elapsed = 0.0f;
};

struct LoopingMushroomEffect : public dynamic::DynamicObject {
  enum class Kind {
    Catch,
    Destroy,
  };

  LoopingMushroomEffect(Kind kind, std::string texture_name, glm::vec2 center, glm::vec2 size,
                        glm::vec2 target_center, float period, float trigger_at)
      : dynamic::DynamicObject(),
        kind(kind),
        texture_name(std::move(texture_name)),
        center(center),
        size(size),
        target_center(target_center),
        period(period),
        trigger_at(trigger_at) {}

  ~LoopingMushroomEffect() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) {
      if (subject && !subject->is_pending_deletion()) subject->mark_deleted();
      subject = nullptr;
      return;
    }
    if (!subject && !triggered) {
      reset_subject();
    }
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += std::max(0.0f, dt);
    if (elapsed >= period) {
      reset_subject();
      return;
    }
    if (!triggered && elapsed >= trigger_at) {
      triggered = true;
      if (!subject || subject->is_pending_deletion()) {
        subject = nullptr;
        return;
      }
      if (kind == Kind::Catch) {
        vfx::spawn_catch_effect(subject, target_center);
        subject = nullptr;
      } else {
        vfx::spawn_destroy_effect(subject);
        subject->mark_deleted();
        subject = nullptr;
      }
    }
  }

  void reset_subject() {
    if (subject && !subject->is_pending_deletion()) subject->mark_deleted();
    subject = create_sprite(texture_name, center, size, 4);
    elapsed = 0.0f;
    triggered = false;
  }

  Kind kind = Kind::Catch;
  std::string texture_name{};
  glm::vec2 center{0.0f, 0.0f};
  glm::vec2 size{0.0f, 0.0f};
  glm::vec2 target_center{0.0f, 0.0f};
  float period = 2.0f;
  float trigger_at = 0.6f;
  float elapsed = 0.0f;
  bool triggered = false;
  ecs::Entity* subject = nullptr;
};

struct MuseumFamiliarMotion : public dynamic::DynamicObject {
  MuseumFamiliarMotion(transform::NoRotationTransform* transform, player::FamiliarSprite* sprite,
                       FamiliarMuseumMode mode, glm::vec2 center, glm::vec2 size,
                       engine::TextureId normal_texture_id, engine::TextureId strike_texture_id)
      : dynamic::DynamicObject(),
        transform(transform),
        sprite(sprite),
        mode(mode),
        center(center),
        size(size),
        normal_texture_id(normal_texture_id),
        strike_texture_id(strike_texture_id) {}

  ~MuseumFamiliarMotion() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion() || !transform || !sprite) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += std::max(0.0f, dt);
    float p = std::fmod(elapsed, period) / period;
    if (p < 0.0f) p += 1.0f;

    switch (mode) {
      case FamiliarMuseumMode::Shoot:
        update_shoot(p);
        break;
      case FamiliarMuseumMode::Trap:
        update_trap(p);
        break;
      case FamiliarMuseumMode::Return:
        update_return(p);
        break;
    }
  }

  void apply_pose(const glm::vec2& next_center, float scale, float rotation,
                  engine::TextureId texture_id, bool idle_wobble) {
    transform->pos = top_left_for_center(next_center, size);
    sprite->texture_id = texture_id;
    sprite->model_scale = std::max(0.001f, scale);
    sprite->model_rotation_rad = rotation;
    sprite->grayscale = false;
    sprite->use_idle_wobble(idle_wobble);
  }

  void update_shoot(float p) {
    if (p < 0.58f) {
      const float t = smooth01(p / 0.58f);
      apply_pose(lerp(center + glm::vec2{0.0f, 92.0f}, center + glm::vec2{0.0f, -118.0f}, t),
                 1.0f, 0.0f, strike_texture_id, false);
      return;
    }
    const float t = smooth01((p - 0.58f) / 0.42f);
    apply_pose(lerp(center + glm::vec2{0.0f, -118.0f}, center + glm::vec2{0.0f, 92.0f}, t),
               1.0f, 3.14159265f, normal_texture_id, false);
  }

  void update_trap(float p) {
    if (p < 0.22f) {
      const float t = smooth01(p / 0.22f);
      apply_pose(lerp(center + glm::vec2{0.0f, 72.0f}, center, t),
                 0.04f + 0.96f * t, 0.0f, normal_texture_id, false);
      return;
    }
    if (p < 0.76f) {
      apply_pose(center, 1.0f, 0.0f, normal_texture_id, true);
      return;
    }
    const float t = smooth01((p - 0.76f) / 0.24f);
    apply_pose(lerp(center, center + glm::vec2{0.0f, 72.0f}, t),
               1.0f + (0.04f - 1.0f) * t, 0.0f, normal_texture_id, false);
  }

  void update_return(float p) {
    const glm::vec2 start = center + glm::vec2{-120.0f, -90.0f};
    const glm::vec2 end = center + glm::vec2{92.0f, 68.0f};
    const glm::vec2 straight = lerp(start, end, smooth01(p));
    const glm::vec2 arc{0.0f, -std::sin(p * 3.14159265f) * 72.0f};
    const glm::vec2 next_center = straight + arc;
    const glm::vec2 tangent = end - start + glm::vec2{0.0f, -std::cos(p * 3.14159265f) * 72.0f};
    const float heading = std::atan2(tangent.y, tangent.x);
    apply_pose(next_center, 1.0f, heading + 1.57079633f, normal_texture_id, false);
  }

  transform::NoRotationTransform* transform = nullptr;
  player::FamiliarSprite* sprite = nullptr;
  FamiliarMuseumMode mode = FamiliarMuseumMode::Shoot;
  glm::vec2 center{0.0f, 0.0f};
  glm::vec2 size{0.0f, 0.0f};
  engine::TextureId normal_texture_id = engine::kInvalidTextureId;
  engine::TextureId strike_texture_id = engine::kInvalidTextureId;
  float period = 2.4f;
  float elapsed = 0.0f;
};

ecs::Entity* create_familiar_demo(const glm::vec2& center, FamiliarMuseumMode mode,
                                  int layer = 4) {
  const glm::vec2 size = player::familiar_display_size() * 2.1f;
  const engine::TextureId normal_tex = engine::resources::register_texture("famiriar");
  const engine::TextureId strike_tex = engine::resources::register_texture("familiar_attack");

  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = top_left_for_center(center, size);
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(layer));
  auto* sprite = arena::create<player::FamiliarSprite>(normal_tex, size);
  player::configure_familiar_sprite(sprite);
  entity->add(sprite);
  entity->add(arena::create<color::OneColor>(glm_color(1.0f, 1.0f, 1.0f, 1.0f)));
  entity->add(arena::create<MuseumFamiliarMotion>(transform, sprite, mode, center, size,
                                                  normal_tex, strike_tex));
  entity->add(arena::create<scene::SceneObject>(kSceneName));
  return entity;
}

void create_effect_controller(LoopingMushroomEffect::Kind kind, std::string texture_name,
                              const glm::vec2& center, const glm::vec2& size,
                              const glm::vec2& target_center, float period,
                              float trigger_at) {
  auto* entity = arena::create<ecs::Entity>();
  entity->add(arena::create<LoopingMushroomEffect>(
      kind, std::move(texture_name), center, size, target_center, period, trigger_at));
  entity->add(arena::create<scene::SceneObject>(kSceneName));
}

void add_section_title(std::string_view title, float x, float y) {
  create_text(std::string{title}, glm::vec2{x, y}, 22.0f,
              glm_color(0.96f, 0.98f, 1.0f, 1.0f), 30);
}

void build_backgrounds_page(float view_w, float view_h) {
  add_section_title("backgrounds", 40.0f, kTopChromePx + 10.0f);

  constexpr int cols = 4;
  const float margin_x = 56.0f;
  const float gap_x = 24.0f;
  const float gap_y = 34.0f;
  const float cell_w = (view_w - margin_x * 2.0f - gap_x * static_cast<float>(cols - 1)) /
                       static_cast<float>(cols);
  const float cell_h = 190.0f;
  const float start_y = kTopChromePx + 58.0f;

  for (size_t i = 0; i < ::shrooms::kBackgroundTextureNames.size(); ++i) {
    const int col = static_cast<int>(i % cols);
    const int row = static_cast<int>(i / cols);
    const float x = margin_x + static_cast<float>(col) * (cell_w + gap_x);
    const float y = start_y + static_cast<float>(row) * (cell_h + gap_y);
    create_rect(glm::vec2{x, y}, glm::vec2{cell_w, cell_h},
                glm_color(0.07f, 0.075f, 0.09f, 0.78f), -8);

    const std::string_view name = ::shrooms::kBackgroundTextureNames[i];
    const glm::vec2 size = fit_texture_size(name, cell_w - 28.0f, cell_h - 42.0f);
    const glm::vec2 center{x + cell_w * 0.5f, y + (cell_h - 24.0f) * 0.48f};
    create_sprite(name, center, size, 0);
    create_caption(name, glm::vec2{x + cell_w * 0.5f, 0.0f}, y + cell_h - 26.0f);
  }

  create_text("Level art, floor strips, and fallback background texture", glm::vec2{40.0f, view_h - 44.0f},
              17.0f, glm_color(0.62f, 0.68f, 0.76f, 1.0f), 30);
}

void build_sprites_page(float view_w, float) {
  add_section_title("sprites", 40.0f, kTopChromePx + 10.0f);

  std::vector<std::string_view> names;
  append_names(names, ::shrooms::kMushroomTextureNames);
  append_names(names, ::shrooms::kPlayerTextureNames);
  append_names(names, ::shrooms::kFamiliarTextureNames);
  append_names(names, ::shrooms::kUiTextureNames);
  append_names(names, ::shrooms::kEmojiTextureNames);

  constexpr int cols = 7;
  const float margin_x = 50.0f;
  const float cell_w = (view_w - margin_x * 2.0f) / static_cast<float>(cols);
  const float cell_h = 136.0f;
  const float start_y = kTopChromePx + 66.0f;

  for (size_t i = 0; i < names.size(); ++i) {
    const int col = static_cast<int>(i % cols);
    const int row = static_cast<int>(i / cols);
    const float x = margin_x + static_cast<float>(col) * cell_w;
    const float y = start_y + static_cast<float>(row) * cell_h;
    const std::string_view name = names[i];
    const glm::vec2 size = fit_texture_size(name, cell_w * 0.62f, 70.0f);
    const glm::vec2 center{x + cell_w * 0.5f, y + 42.0f};
    create_sprite(name, center, size, 2);
    create_caption(name, glm::vec2{x + cell_w * 0.5f, 0.0f}, y + 86.0f);
  }
}

void build_player_page(float view_w, float) {
  add_section_title("player animations", 40.0f, kTopChromePx + 10.0f);

  const glm::vec2 size = player::player_display_size() * 2.35f;
  const float y = kTopChromePx + 255.0f;
  const std::array<float, 4> xs{
      view_w * 0.17f,
      view_w * 0.39f,
      view_w * 0.61f,
      view_w * 0.83f,
  };

  if (auto* idle_left = create_sprite("witch_left_1", glm::vec2{xs[0], y}, size, 2)) {
    idle_left->add(arena::create<animation::SpriteAnimation>(
        player::make_player_animation_clips(), "idle_left"));
  }
  create_caption("idle left", glm::vec2{xs[0], 0.0f}, y + size.y * 0.72f);

  if (auto* idle_right = create_sprite("witch_right_1", glm::vec2{xs[1], y}, size, 2)) {
    idle_right->add(arena::create<animation::SpriteAnimation>(
        player::make_player_animation_clips(), "idle_right"));
  }
  create_caption("idle right", glm::vec2{xs[1], 0.0f}, y + size.y * 0.72f);

  auto* left = create_sprite("witch_left_1", glm::vec2{xs[2] + 42.0f, y}, size, 2);
  if (left) {
    left->add(arena::create<animation::SpriteAnimation>(player::make_player_animation_clips(),
                                                        "left"));
    auto* transform = left->get<transform::NoRotationTransform>();
    if (transform) {
      left->add(arena::create<LoopingSlide>(
          transform, top_left_for_center(glm::vec2{xs[2] + 58.0f, y}, size),
          top_left_for_center(glm::vec2{xs[2] - 58.0f, y}, size), 2.0f));
    }
  }
  create_caption("go left", glm::vec2{xs[2], 0.0f}, y + size.y * 0.72f);

  auto* right = create_sprite("witch_right_1", glm::vec2{xs[3] - 42.0f, y}, size, 2);
  if (right) {
    right->add(arena::create<animation::SpriteAnimation>(player::make_player_animation_clips(),
                                                         "right"));
    auto* transform = right->get<transform::NoRotationTransform>();
    if (transform) {
      right->add(arena::create<LoopingSlide>(
          transform, top_left_for_center(glm::vec2{xs[3] - 58.0f, y}, size),
          top_left_for_center(glm::vec2{xs[3] + 58.0f, y}, size), 2.0f));
    }
  }
  create_caption("go right", glm::vec2{xs[3], 0.0f}, y + size.y * 0.72f);
}

void build_mushroom_page(float view_w, float) {
  add_section_title("mushroom animations", 40.0f, kTopChromePx + 10.0f);

  const glm::vec2 size = mushroom_display_size("mukhomor") * 1.25f;
  const float y = kTopChromePx + 235.0f;
  const std::array<float, 3> xs{view_w * 0.24f, view_w * 0.50f, view_w * 0.76f};

  auto* falling = create_sprite("mukhomor", glm::vec2{xs[0], y - 65.0f}, size, 3);
  if (falling) {
    auto* transform = falling->get<transform::NoRotationTransform>();
    auto* tint = falling->get<color::OneColor>();
    falling->add(arena::create<LoopingMushroomFall>(
        transform, tint, glm::vec2{xs[0], y - 98.0f}, size, 230.0f, 2.6f));
  }
  create_caption("fall", glm::vec2{xs[0], 0.0f}, y + 145.0f);

  create_effect_controller(LoopingMushroomEffect::Kind::Catch, "lisi4ka",
                           glm::vec2{xs[1], y + 18.0f}, mushroom_display_size("lisi4ka") * 1.25f,
                           glm::vec2{xs[1], y - 100.0f}, 2.2f, 0.55f);
  create_caption("collected", glm::vec2{xs[1], 0.0f}, y + 145.0f);

  create_effect_controller(LoopingMushroomEffect::Kind::Destroy, "borovik",
                           glm::vec2{xs[2], y + 12.0f}, mushroom_display_size("borovik") * 1.25f,
                           glm::vec2{xs[2], y}, 2.35f, 0.58f);
  create_caption("shot", glm::vec2{xs[2], 0.0f}, y + 145.0f);
}

void build_bat_page(float view_w, float) {
  add_section_title("bat animations", 40.0f, kTopChromePx + 10.0f);

  const float y = kTopChromePx + 255.0f;
  const std::array<float, 3> xs{view_w * 0.25f, view_w * 0.50f, view_w * 0.75f};
  create_familiar_demo(glm::vec2{xs[0], y}, FamiliarMuseumMode::Shoot);
  create_caption("shoot", glm::vec2{xs[0], 0.0f}, y + 140.0f);

  create_familiar_demo(glm::vec2{xs[1], y}, FamiliarMuseumMode::Trap);
  create_caption("trap", glm::vec2{xs[1], 0.0f}, y + 140.0f);

  create_familiar_demo(glm::vec2{xs[2], y}, FamiliarMuseumMode::Return);
  create_caption("return", glm::vec2{xs[2], 0.0f}, y + 140.0f);
}

void build_hud_page(float view_w, float) {
  add_section_title("hud samples", 40.0f, kTopChromePx + 10.0f);

  const float center_y = kTopChromePx + 335.0f;
  const glm::vec4 label_color = glm_color(0.82f, 0.86f, 0.92f, 0.95f);
  const glm::vec4 text_color = glm_color(0.98f, 0.98f, 1.0f, 1.0f);
  const glm::vec4 muted_tint = glm_color(0.36f, 0.36f, 0.40f, 0.82f);

  const glm::vec2 face_panel_center{view_w * 0.30f, center_y};
  const glm::vec2 face_panel_size = fit_texture_size("menu_face", 245.0f, 230.0f);
  create_sprite("menu_face", face_panel_center, face_panel_size, 1);

  const glm::vec2 bat_size = fit_texture_size("bat_face", 38.0f, 34.0f);
  const float bat_gap = 9.0f;
  const float bat_row_width = bat_size.x * 3.0f + bat_gap * 2.0f;
  const glm::vec2 bat_row_start{
      face_panel_center.x - bat_row_width * 0.5f + bat_size.x * 0.5f,
      face_panel_center.y - face_panel_size.y * 0.28f,
  };
  for (int i = 0; i < 3; ++i) {
    const glm::vec4 tint = i < 2 ? glm::vec4{1.0f} : muted_tint;
    create_sprite("bat_face",
                  bat_row_start + glm::vec2{static_cast<float>(i) * (bat_size.x + bat_gap),
                                            0.0f},
                  bat_size, 3, tint);
  }

  create_centered_text("127",
                       face_panel_center + glm::vec2{0.0f, -face_panel_size.y * 0.04f},
                       24.0f, text_color, 4);

  const glm::vec2 face_size = fit_texture_size("face_mini_1", 112.0f, 66.0f);
  auto* face = create_sprite("face_mini_1",
                             face_panel_center + glm::vec2{0.0f, face_panel_size.y * 0.22f},
                             face_size, 4);
  add_face_animation(face);
  create_caption("animated player face", glm::vec2{face_panel_center.x, 0.0f},
                 face_panel_center.y + face_panel_size.y * 0.62f);

  const glm::vec2 board_center{view_w * 0.70f, center_y};
  const glm::vec2 board_size = fit_texture_size("menu_scoreboard", 270.0f, 365.0f);
  create_sprite("menu_scoreboard", board_center, board_size, 1);
  create_centered_text("127", board_center + glm::vec2{0.0f, -board_size.y * 0.34f},
                       25.0f, text_color, 4);

  struct ScoreboardRow {
    std::string_view texture_name;
    std::string_view value;
  };
  constexpr std::array<ScoreboardRow, 3> rows{{
      {"borovik_small", "2/3"},
      {"mukhomor_small", "1/2"},
      {"lisi4ka_small", "0/1"},
  }};

  const float row_gap = board_size.y * 0.15f;
  const float row_start_y = board_center.y - row_gap * 0.45f;
  for (size_t i = 0; i < rows.size(); ++i) {
    const auto& row = rows[i];
    const glm::vec2 row_center{board_center.x, row_start_y + static_cast<float>(i) * row_gap};
    const glm::vec2 icon_size = fit_texture_size(row.texture_name, 42.0f, 42.0f);
    const float font_px = 23.0f;
    const std::string value{row.value};
    const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, font_px);
    const float gap = 14.0f;
    const float total_width = icon_size.x + gap + layout.width;
    const float left = board_center.x - total_width * 0.5f;

    create_sprite(row.texture_name, glm::vec2{left + icon_size.x * 0.5f, row_center.y},
                  icon_size, 4);
    create_text(value, glm::vec2{left + icon_size.x + gap,
                                 row_center.y - layout.height * 0.5f},
                font_px, text_color, 4);
  }

  create_centered_text("sample scoreboard",
                       glm::vec2{board_center.x, board_center.y + board_size.y * 0.60f},
                       14.0f, label_color, 20);
}

void build_font_page(float view_w, float view_h) {
  add_section_title("font", 40.0f, kTopChromePx + 10.0f);

  const engine::TextureId atlas_id = engine::text::atlas_texture_id();
  const glm::vec2 atlas_size{
      static_cast<float>(engine::text::atlas_width()) * 3.0f,
      static_cast<float>(engine::text::atlas_height()) * 3.0f,
  };
  create_sprite(atlas_id, glm::vec2{view_w * 0.20f, kTopChromePx + 168.0f}, atlas_size, 2);
  create_caption("engine_font_atlas", glm::vec2{view_w * 0.20f, 0.0f},
                 kTopChromePx + 168.0f + atlas_size.y * 0.58f);

  const std::string glyphs =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!?+-*/";
  const int cols = 18;
  const float font_px = 24.0f;
  const float start_x = view_w * 0.42f;
  const float start_y = kTopChromePx + 82.0f;
  const float step_x = 32.0f;
  const float step_y = 34.0f;
  for (size_t i = 0; i < glyphs.size(); ++i) {
    const int col = static_cast<int>(i % cols);
    const int row = static_cast<int>(i / cols);
    create_text(std::string(1, glyphs[i]),
                glm::vec2{start_x + static_cast<float>(col) * step_x,
                          start_y + static_cast<float>(row) * step_y},
                font_px, glm_color(0.94f, 0.98f, 1.0f, 1.0f), 4);
  }

  create_text("The quick brown fox jumps over the lazy dog 0123456789",
              glm::vec2{70.0f, view_h - 142.0f}, 30.0f,
              glm_color(0.98f, 0.98f, 1.0f, 1.0f), 4);
  create_text("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG",
              glm::vec2{70.0f, view_h - 92.0f}, 24.0f,
              glm_color(0.70f, 0.78f, 0.9f, 1.0f), 4);
}

struct MuseumController : public dynamic::DynamicObject {
  MuseumController(int view_width, int view_height)
      : dynamic::DynamicObject(-100), view_width(view_width), view_height(view_height) {}

  ~MuseumController() override { Component::component_count--; }

  void update() override {
    const int next_page = page_index_for_time(ecs::context().time_seconds);
    if (next_page == active_page) return;
    active_page = next_page;
    rebuild_page();
  }

  void rebuild_page() const {
    clear_scene_entities();
    const float w = static_cast<float>(view_width);
    const float h = static_cast<float>(view_height);
    create_rect(glm::vec2{0.0f, kTopChromePx}, glm::vec2{w, h - kTopChromePx},
                glm_color(0.025f, 0.027f, 0.034f, 1.0f), -20);
    switch (active_page) {
      case 0:
        build_backgrounds_page(w, h);
        break;
      case 1:
        build_sprites_page(w, h);
        break;
      case 2:
        build_player_page(w, h);
        break;
      case 3:
        build_mushroom_page(w, h);
        break;
      case 4:
        build_bat_page(w, h);
        break;
      case 5:
        build_hud_page(w, h);
        break;
      case 6:
      default:
        build_font_page(w, h);
        break;
    }
  }

  int view_width = 0;
  int view_height = 0;
  int active_page = -1;
};

void draw_tabs(engine::Frame& frame, double time_seconds, float view_w) {
  const int active = page_index_for_time(time_seconds);
  const float progress = page_time(time_seconds) / kTabSeconds;
  const float tab_w = view_w / static_cast<float>(kPageNames.size());

  engine::add_rect(frame.ui, 0.0f, 0.0f, view_w, kTopChromePx,
                   ui_color(0.035f, 0.038f, 0.048f, 0.96f));
  engine::add_text(frame.ui, "Shrooms Art Museum", 22.0f, 14.0f, 24.0f,
                   ui_color(0.95f, 0.97f, 1.0f, 1.0f));

  for (size_t i = 0; i < kPageNames.size(); ++i) {
    const bool is_active = static_cast<int>(i) == active;
    const float x = static_cast<float>(i) * tab_w;
    const engine::UIColor tab_color =
        is_active ? ui_color(0.19f, 0.24f, 0.31f, 0.98f)
                  : ui_color(0.075f, 0.082f, 0.10f, 0.92f);
    engine::add_rect(frame.ui, x + 2.0f, 52.0f, tab_w - 4.0f, 30.0f, tab_color);
    engine::add_text(frame.ui, std::string{kPageNames[i]}, x + 12.0f, 60.0f, 13.0f,
                     is_active ? ui_color(1.0f, 1.0f, 1.0f, 1.0f)
                               : ui_color(0.70f, 0.74f, 0.80f, 1.0f));
  }

  engine::add_rect(frame.ui, 0.0f, kTopChromePx - 4.0f, view_w, 4.0f,
                   ui_color(0.08f, 0.09f, 0.12f, 1.0f));
  engine::add_rect(frame.ui, static_cast<float>(active) * tab_w, kTopChromePx - 4.0f,
                   tab_w * progress, 4.0f, ui_color(0.65f, 0.78f, 1.0f, 1.0f));
}

}  // namespace

MuseumLogic::MuseumLogic(int view_width, int view_height)
    : view_width_(view_width), view_height_(view_height) {}

void MuseumLogic::on_init() {
  ::shrooms::screen::set_view(view_width_, view_height_);
  ::shrooms::register_shrooms_svg_assets();
  engine::text::atlas_texture_id();

  auto* main_scene = scene::ensure_scene(kSceneName);
  main_scene->activate();
  main_scene->set_pause(false);

  auto* controller = arena::create<ecs::Entity>();
  controller->add(arena::create<MuseumController>(view_width_, view_height_));

  auto* system_entity = arena::create<ecs::Entity>();
  system_entity->add(arena::create<animation::Animation>());
  system_entity->add(arena::create<render_system::RenderSystem>());
}

void MuseumLogic::after_tick(const engine::AppContext& ctx,
                             std::span<const engine::InputEvent>,
                             engine::Frame& frame) {
  draw_tabs(frame, ctx.time_seconds, static_cast<float>(view_width_));
}

}  // namespace engine::shrooms
