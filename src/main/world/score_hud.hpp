#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "ecs/context.hpp"
#include "utils/arena.hpp"
#include "systems/animation/sprite_animation.hpp"
#include "systems/color/color_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/hidden/hidden_object.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"
#include "engine/geometry_builder.h"

#include "scoreboard.hpp"
#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"

namespace score_hud {

struct Config {
  glm::vec2 panel_reference_size = glm::vec2(0.0f, 0.0f);
  glm::vec2 face_reference_size = glm::vec2(0.0f, 0.0f);
  glm::vec2 face_offset = glm::vec2(0.0f, 0.0f);
  glm::vec2 score_offset = glm::vec2(0.0f, 0.0f);
  glm::vec2 lives_offset = glm::vec2(0.0f, 0.0f);
  float score_font_px = 20.0f;
  glm::vec4 score_color = glm::vec4(1.0f);
  float intro_slide_duration = 0.65f;
  float intro_slide_pad_px = 18.0f;
  int layer = 1;
} config;

inline constexpr size_t kMaxLifeHearts = 3;
inline constexpr size_t kMaxBatIcons = 3;
inline constexpr float kTopRowCenterYFraction = 0.22f;
inline constexpr float kHeartRowCenterYFraction = 0.40f;
inline constexpr float kFaceCenterXFraction = 0.50f;
inline constexpr float kFaceCenterYFraction = 0.70f;
inline constexpr float kHeartWidthFraction = 0.024f;
inline constexpr float kRowGapToHeartWidth = 0.18f;
inline constexpr float kScoreBatGapToHeartWidth = 0.42f;
inline constexpr float kBatRowMaxWidthFraction = 0.17f;
inline constexpr float kBatHeightToHeartHeight = 0.82f;

inline ecs::Entity* face_icon = nullptr;
inline ecs::Entity* panel = nullptr;
inline transform::NoRotationTransform* panel_transform = nullptr;
inline transform::NoRotationTransform* face_transform = nullptr;
inline ecs::Entity* score_text_entity = nullptr;
inline transform::NoRotationTransform* score_text_transform = nullptr;
inline text::TextObject* score_text = nullptr;
inline color::OneColor* score_text_color = nullptr;
inline std::array<ecs::Entity*, kMaxLifeHearts> life_heart_entities{};
inline std::array<transform::NoRotationTransform*, kMaxLifeHearts> life_heart_transforms{};
inline std::array<render_system::SpriteRenderable*, kMaxLifeHearts> life_heart_sprites{};
inline std::array<hidden::HiddenObject*, kMaxLifeHearts> life_heart_hidden{};
inline std::array<color::OneColor*, kMaxLifeHearts> life_heart_colors{};
inline std::array<ecs::Entity*, kMaxBatIcons> bat_icon_entities{};
inline std::array<transform::NoRotationTransform*, kMaxBatIcons> bat_icon_transforms{};
inline std::array<render_system::SpriteRenderable*, kMaxBatIcons> bat_icon_sprites{};
inline std::array<color::OneColor*, kMaxBatIcons> bat_icon_colors{};
inline int current_score = 0;
inline int current_lives = 0;
inline int current_ready_bats = static_cast<int>(kMaxBatIcons);
inline bool lives_visible = false;
inline glm::vec2 hud_offset_px{0.0f, 0.0f};
inline glm::vec2 hud_anim_start_px{0.0f, 0.0f};
inline glm::vec2 hud_anim_target_px{0.0f, 0.0f};
inline float hud_anim_elapsed = 0.0f;
inline float hud_anim_duration = 0.0f;
inline bool hud_anim_active = false;

inline float clamp01(float value) {
  return std::min(1.0f, std::max(0.0f, value));
}

inline float ease_out(float value) {
  const float t = clamp01(value);
  const float inv = 1.0f - t;
  return 1.0f - inv * inv;
}

inline glm::vec2 resolve_reference_size(const glm::vec2& configured,
                                        std::string_view texture_name,
                                        float fallback_width) {
  if (configured.x > 0.0f && configured.y > 0.0f) return configured;
  const glm::vec2 measured = shrooms::texture_sizing::reference_size(texture_name);
  if (measured.x > 0.0f && measured.y > 0.0f) return measured;
  return shrooms::texture_sizing::reference_size_from_width(texture_name, fallback_width);
}

inline glm::vec2 panel_top_left_norm() { return glm::vec2{-1.0f, 1.0f}; }

inline glm::vec2 panel_size_px() {
  const glm::vec2 ref_size = resolve_reference_size(config.panel_reference_size, "menu_face", 107.0f);
  return shrooms::texture_sizing::from_reference_size(ref_size);
}

inline glm::vec2 panel_center_px() {
  const glm::vec2 top_left = shrooms::screen::norm_to_pixels(panel_top_left_norm());
  return top_left + panel_size_px() * 0.5f;
}

inline glm::vec2 panel_top_left_px() {
  return shrooms::screen::norm_to_pixels(panel_top_left_norm());
}

inline glm::vec2 panel_center_norm() {
  const glm::vec2 size = panel_size_px();
  const glm::vec2 top_left_norm = panel_top_left_norm();
  return glm::vec2{
      top_left_norm.x + (size.x / static_cast<float>(shrooms::screen::view_width)),
      top_left_norm.y - (size.y / static_cast<float>(shrooms::screen::view_height)),
  };
}

inline glm::vec2 face_size_px() {
  const glm::vec2 ref_size =
      resolve_reference_size(config.face_reference_size, "face_mini_1", 24.0f);
  return shrooms::texture_sizing::from_reference_size(ref_size);
}

inline glm::vec2 heart_size_px() {
  const float heart_width =
      shrooms::screen::scale_to_pixels(glm::vec2{kHeartWidthFraction, 0.0f}).x;
  return shrooms::texture_sizing::from_width_px("heart", heart_width);
}

inline glm::vec2 bat_size_px() {
  const glm::vec2 heart_size = heart_size_px();
  const float aspect = std::max(0.1f, shrooms::texture_sizing::aspect_ratio("famiriar"));
  const float target_width = heart_size.y * kBatHeightToHeartHeight * aspect;
  const float max_width = panel_size_px().x * kBatRowMaxWidthFraction;
  const float bat_width = std::max(11.0f, std::min(max_width, target_width));
  return shrooms::texture_sizing::from_width_px("famiriar", bat_width);
}

inline float row_gap_px(const glm::vec2& heart_size) {
  return std::max(2.0f, heart_size.x * kRowGapToHeartWidth);
}

inline float score_bat_gap_px(const glm::vec2& heart_size) {
  return std::max(4.0f, heart_size.x * kScoreBatGapToHeartWidth);
}

inline glm::vec2 row_center_px(float y_fraction) {
  const glm::vec2 top_left = panel_top_left_px();
  const glm::vec2 size = panel_size_px();
  return top_left + glm::vec2{size.x * 0.5f, size.y * y_fraction} + hud_offset_px;
}

inline glm::vec2 score_text_size_px(std::string_view value) {
  const auto layout =
      engine::text::layout_text(std::string(value), 0.0f, 0.0f, config.score_font_px);
  return glm::vec2{layout.width, layout.height};
}

inline float bat_row_width_px(const glm::vec2& bat_size, float gap) {
  return bat_size.x * static_cast<float>(kMaxBatIcons) +
         gap * static_cast<float>(kMaxBatIcons - 1);
}

inline float heart_row_width_px(const glm::vec2& heart_size, float gap) {
  return heart_size.x * static_cast<float>(kMaxLifeHearts) +
         gap * static_cast<float>(kMaxLifeHearts - 1);
}

inline glm::vec2 score_anchor_px() {
  return scoreboard::score_anchor_px();
}

inline glm::vec2 face_center_px() {
  const glm::vec2 top_left = panel_top_left_px();
  const glm::vec2 size = panel_size_px();
  return top_left + glm::vec2{size.x * kFaceCenterXFraction,
                              size.y * kFaceCenterYFraction} +
         shrooms::screen::scale_to_pixels(config.face_offset) +
         hud_offset_px;
}

inline glm::vec2 lives_anchor_px() {
  return row_center_px(kHeartRowCenterYFraction) +
         shrooms::screen::scale_to_pixels(config.lives_offset);
}

inline glm::vec2 heart_row_start_px(const glm::vec2& heart_size, float gap) {
  const float total_width = heart_row_width_px(heart_size, gap);
  return lives_anchor_px() - glm::vec2{total_width * 0.5f, 0.0f};
}

inline glm::vec2 bat_row_start_px(const glm::vec2& bat_size, float icon_gap) {
  const float total_width = bat_row_width_px(bat_size, icon_gap);
  return row_center_px(kTopRowCenterYFraction) -
         glm::vec2{total_width * 0.5f, 0.0f} +
         shrooms::screen::scale_to_pixels(config.score_offset);
}

inline void update_score_layout() {}

inline void update_lives_layout() {}

inline void update_bat_layout() {
  const glm::vec2 heart_size = heart_size_px();
  const glm::vec2 bat_size = bat_size_px();
  const float gap = row_gap_px(heart_size);
  const glm::vec2 row_start = bat_row_start_px(bat_size, gap);
  const int ready_bats = std::clamp(current_ready_bats, 0, static_cast<int>(kMaxBatIcons));

  for (size_t i = 0; i < kMaxBatIcons; ++i) {
    if (bat_icon_transforms[i]) {
      const glm::vec2 center =
          row_start + glm::vec2{bat_size.x * 0.5f +
                                    static_cast<float>(i) * (bat_size.x + gap),
                                0.0f};
      bat_icon_transforms[i]->pos = shrooms::screen::center_to_top_left(center, bat_size);
    }
    if (bat_icon_sprites[i]) {
      bat_icon_sprites[i]->size = bat_size;
      bat_icon_sprites[i]->geometry = engine::geometry::make_quad(bat_size.x, bat_size.y);
      bat_icon_sprites[i]->uploaded = false;
    }
    if (bat_icon_colors[i]) {
      const bool ready = static_cast<int>(i) < ready_bats;
      bat_icon_colors[i]->color =
          ready ? glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}
                : glm::vec4{0.32f, 0.32f, 0.36f, 0.72f};
    }
  }
}

inline void update_panel_layout() {
  const glm::vec2 panel_top_left = panel_top_left_px();
  const glm::vec2 panel_size = panel_size_px();
  const glm::vec2 panel_center = panel_top_left + panel_size * 0.5f + hud_offset_px;

  if (panel_transform) {
    panel_transform->pos = shrooms::screen::center_to_top_left(panel_center, panel_size);
  }
  if (face_transform) {
    const glm::vec2 size = face_size_px();
    face_transform->pos = shrooms::screen::center_to_top_left(face_center_px(), size);
  }
  update_score_layout();
  update_lives_layout();
  update_bat_layout();
}

inline void set_score(int score_value) {
  current_score = score_value;
  scoreboard::set_score(score_value);
}

inline int score() { return current_score; }

inline void set_lives(int lives_value) {
  current_lives = lives_value;
  scoreboard::set_hearts(lives_value);
}

inline void set_lives_visible(bool visible) {
  lives_visible = visible;
  scoreboard::set_hearts_visible(visible);
}

inline void set_bat_availability(int ready_count) {
  current_ready_bats = std::clamp(ready_count, 0, static_cast<int>(kMaxBatIcons));
  update_bat_layout();
}

inline void animate_offset_to(const glm::vec2& target, float duration) {
  hud_anim_start_px = hud_offset_px;
  hud_anim_target_px = target;
  hud_anim_elapsed = 0.0f;
  hud_anim_duration = std::max(0.0f, duration);
  hud_anim_active = hud_anim_duration > 0.0f;
  if (!hud_anim_active) {
    hud_offset_px = target;
  }
  update_panel_layout();
}

inline void start_intro_slide_in(float duration = config.intro_slide_duration) {
  hud_offset_px = glm::vec2{-(panel_size_px().x + config.intro_slide_pad_px), 0.0f};
  update_panel_layout();
  animate_offset_to(glm::vec2{0.0f, 0.0f}, duration);
}

struct ScoreHudController : public dynamic::DynamicObject {
  ScoreHudController() : dynamic::DynamicObject() {}
  ~ScoreHudController() override { Component::component_count--; }

  void update() override {
    if (!hud_anim_active) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    hud_anim_elapsed += dt;
    const float t =
        hud_anim_duration > 0.0f ? clamp01(hud_anim_elapsed / hud_anim_duration) : 1.0f;
    const float eased = ease_out(t);
    hud_offset_px = hud_anim_start_px + (hud_anim_target_px - hud_anim_start_px) * eased;
    if (t >= 1.0f) {
      hud_anim_active = false;
      hud_offset_px = hud_anim_target_px;
    }
    update_panel_layout();
  }
};

inline ScoreHudController controller{};

inline void reset_hud() {
  const glm::vec2 panel_top_left = panel_top_left_px();
  const glm::vec2 panel_size = panel_size_px();
  const glm::vec2 panel_center = panel_top_left + panel_size * 0.5f;

  if (panel) {
    panel->mark_deleted();
  }
  panel = arena::create<ecs::Entity>();
  {
    panel_transform = arena::create<transform::NoRotationTransform>();
    panel_transform->pos =
        shrooms::screen::center_to_top_left(panel_center + hud_offset_px, panel_size);
    panel->add(panel_transform);
    panel->add(arena::create<layers::ConstLayer>(config.layer));
    const engine::TextureId tex_id = engine::resources::register_texture("menu_face");
    panel->add(arena::create<render_system::SpriteRenderable>(tex_id, panel_size));
    panel->add(arena::create<scene::SceneObject>("main"));
  }

  if (face_icon) {
    face_icon->mark_deleted();
  }
  face_icon = arena::create<ecs::Entity>();
  {
    const glm::vec2 size = face_size_px();
    face_transform = arena::create<transform::NoRotationTransform>();
    face_transform->pos = shrooms::screen::center_to_top_left(face_center_px(), size);
    face_icon->add(face_transform);
    face_icon->add(arena::create<layers::ConstLayer>(config.layer));
    const engine::TextureId frame_1 = engine::resources::register_texture("face_mini_1");
    const engine::TextureId frame_2 = engine::resources::register_texture("face_mini_2");
    face_icon->add(arena::create<render_system::SpriteRenderable>(frame_1, size));
    std::map<std::string, std::vector<animation::SpriteFrame>> clips{};
    clips["idle"] = {animation::SpriteFrame{frame_1, 0.25f},
                     animation::SpriteFrame{frame_2, 0.25f}};
    face_icon->add(arena::create<animation::SpriteAnimation>(std::move(clips), "idle"));
    face_icon->add(arena::create<scene::SceneObject>("main"));
  }

  if (score_text_entity) {
    score_text_entity->mark_deleted();
  }
  score_text_entity = nullptr;
  score_text_transform = nullptr;
  score_text = nullptr;
  score_text_color = nullptr;

  for (size_t i = 0; i < kMaxLifeHearts; ++i) {
    if (life_heart_entities[i]) {
      life_heart_entities[i]->mark_deleted();
    }
    life_heart_entities[i] = nullptr;
    life_heart_transforms[i] = nullptr;
    life_heart_sprites[i] = nullptr;
    life_heart_hidden[i] = nullptr;
    life_heart_colors[i] = nullptr;
  }

  for (size_t i = 0; i < kMaxBatIcons; ++i) {
    if (bat_icon_entities[i]) {
      bat_icon_entities[i]->mark_deleted();
    }
    bat_icon_entities[i] = arena::create<ecs::Entity>();
    bat_icon_transforms[i] = arena::create<transform::NoRotationTransform>();
    bat_icon_entities[i]->add(bat_icon_transforms[i]);
    bat_icon_entities[i]->add(arena::create<layers::ConstLayer>(config.layer + 1));
    const engine::TextureId tex_id = engine::resources::register_texture("famiriar");
    const glm::vec2 size = shrooms::texture_sizing::from_width_px("famiriar", 14.0f);
    bat_icon_sprites[i] = arena::create<render_system::SpriteRenderable>(tex_id, size);
    bat_icon_entities[i]->add(bat_icon_sprites[i]);
    bat_icon_colors[i] = arena::create<color::OneColor>(glm::vec4{1.0f});
    bat_icon_entities[i]->add(bat_icon_colors[i]);
    bat_icon_entities[i]->add(arena::create<scene::SceneObject>("main"));
  }
  update_bat_layout();
}

inline void init() {
  current_score = 0;
  current_lives = 0;
  current_ready_bats = static_cast<int>(kMaxBatIcons);
  lives_visible = false;
  scoreboard::set_score(0);
  scoreboard::set_hearts(0);
  scoreboard::set_hearts_visible(false);
  hud_offset_px = glm::vec2{0.0f, 0.0f};
  hud_anim_active = false;
  reset_hud();
}

inline void reset_for_run() { set_score(0); }

}  // namespace score_hud
