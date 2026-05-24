#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "ecs/context.hpp"
#include "utils/arena.hpp"
#include "systems/color/color_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"
#include "engine/geometry_builder.h"

#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"

namespace scoreboard {

struct Config {
  glm::vec2 icon_scale = glm::vec2(0.05f, 0.05f);
  float row_spacing = 0.115f;
  float row_offset_x_px = 8.0f;
  float row_icon_gap_px = 10.0f;
  glm::vec2 center_intro_norm = glm::vec2{0.0f, -0.18f};
  float default_move_duration = 0.65f;
  float shake_duration = 0.45f;
  float shake_amplitude_px = 10.0f;
  float shake_frequency = 38.0f;
  float text_font_px = 20.0f;
  float score_font_px = 20.0f;
  glm::vec4 text_color = glm::vec4(1.0f);
  int layer = 1;
} config;

enum class LayoutState {
  Hidden,
  CenterIntro,
  Corner,
};

struct Entry {
  std::string name;
  int current = 0;
  int target = 0;
  ecs::Entity* icon = nullptr;
  transform::NoRotationTransform* icon_transform = nullptr;
  ecs::Entity* score_text_entity = nullptr;
  transform::NoRotationTransform* score_text_transform = nullptr;
  text::TextObject* score_text = nullptr;
  size_t row_index = 0;
};

inline constexpr size_t kMaxHeartIcons = 3;
inline constexpr float kHeartWidthFraction = 0.024f;
inline constexpr float kHeartGapToWidth = 0.18f;

inline std::vector<Entry> entries{};
inline std::unordered_map<std::string, size_t> entry_index{};
inline std::vector<std::pair<std::string, int>> current_recipe{};
inline std::string objective_word{};
inline ecs::Entity* panel = nullptr;
inline transform::NoRotationTransform* panel_transform = nullptr;
inline ecs::Entity* score_text_entity = nullptr;
inline transform::NoRotationTransform* score_text_transform = nullptr;
inline text::TextObject* score_text = nullptr;
inline std::array<ecs::Entity*, kMaxHeartIcons> heart_entities{};
inline std::array<transform::NoRotationTransform*, kMaxHeartIcons> heart_transforms{};
inline std::array<render_system::SpriteRenderable*, kMaxHeartIcons> heart_sprites{};
inline std::array<color::OneColor*, kMaxHeartIcons> heart_colors{};
inline size_t active_rows = 0;
inline int current_score = 0;
inline int current_hearts = 0;
inline bool hearts_visible = false;
inline bool status_panel_visible = false;
inline LayoutState layout_state = LayoutState::Corner;
inline glm::vec2 current_panel_center_norm{0.0f, 0.0f};
inline glm::vec2 animation_start_norm{0.0f, 0.0f};
inline glm::vec2 animation_target_norm{0.0f, 0.0f};
inline float animation_elapsed = 0.0f;
inline float animation_duration = 0.0f;
inline bool animation_active = false;
inline float shake_elapsed = 0.0f;
inline float shake_duration = 0.0f;
inline float active_shake_amplitude_px = 0.0f;
inline glm::vec2 shake_offset_px{0.0f, 0.0f};

inline float clamp01(float value) {
  return std::min(1.0f, std::max(0.0f, value));
}

inline float ease_in_out(float value) {
  const float t = clamp01(value);
  return t * t * (3.0f - 2.0f * t);
}

inline glm::vec2 panel_reference_size() {
  const glm::vec2 measured = shrooms::texture_sizing::reference_size("menu_scoreboard");
  if (measured.x > 0.0f && measured.y > 0.0f) return measured;
  return shrooms::texture_sizing::reference_size_from_width("menu_scoreboard", 64.0f);
}

inline glm::vec2 panel_size_px() {
  return shrooms::texture_sizing::from_reference_size(panel_reference_size());
}

inline glm::vec2 corner_panel_center_norm() {
  const glm::vec2 size = panel_size_px();
  return glm::vec2{
      1.0f - (size.x / static_cast<float>(shrooms::screen::view_width)),
      1.0f - (size.y / static_cast<float>(shrooms::screen::view_height)),
  };
}

inline glm::vec2 target_center_norm(LayoutState state) {
  switch (state) {
    case LayoutState::Hidden:
    case LayoutState::Corner:
      return corner_panel_center_norm();
    case LayoutState::CenterIntro:
      return config.center_intro_norm;
  }
  return corner_panel_center_norm();
}

inline glm::vec2 panel_center_norm() {
  if (current_panel_center_norm.x == 0.0f && current_panel_center_norm.y == 0.0f &&
      layout_state == LayoutState::Corner) {
    current_panel_center_norm = corner_panel_center_norm();
  }
  return current_panel_center_norm;
}

inline glm::vec2 current_panel_center_px() {
  return shrooms::screen::norm_to_pixels(panel_center_norm()) + shake_offset_px;
}

inline float layout_row_spacing_px() {
  return config.row_spacing * static_cast<float>(shrooms::screen::view_height) * 0.5f;
}

inline size_t layout_row_count() {
  return 1 + (hearts_visible ? 1 : current_recipe.size());
}

inline glm::vec2 layout_row_center_px(size_t row_index) {
  const size_t rows = std::max<size_t>(1, layout_row_count());
  const float middle = static_cast<float>(rows - 1) * 0.5f;
  const glm::vec2 center = current_panel_center_px();
  return glm::vec2{
      center.x + config.row_offset_x_px,
      center.y + (static_cast<float>(row_index) - middle) * layout_row_spacing_px(),
  };
}

inline glm::vec2 score_anchor_px() {
  return layout_row_center_px(0);
}

inline glm::vec2 heart_size_px() {
  const float heart_width =
      shrooms::screen::scale_to_pixels(glm::vec2{kHeartWidthFraction, 0.0f}).x;
  return shrooms::texture_sizing::from_width_px("heart", heart_width);
}

inline float heart_gap_px(const glm::vec2& heart_size) {
  return std::max(2.0f, heart_size.x * kHeartGapToWidth);
}

inline float heart_row_width_px(const glm::vec2& heart_size, float gap) {
  return heart_size.x * static_cast<float>(kMaxHeartIcons) +
         gap * static_cast<float>(kMaxHeartIcons - 1);
}

inline std::string icon_texture_name(const std::string& name) {
  if (name == "mukhomor" || name == "lisi4ka" || name == "borovik") {
    return name + "_small";
  }
  return name;
}

inline std::string recipe_score_text_value(const Entry& entry) {
  const std::string counter = std::to_string(entry.current) + "/" + std::to_string(entry.target);
  if (objective_word.empty()) return counter;
  return objective_word + "\n" + counter;
}

inline void destroy_entry(Entry& entry) {
  if (entry.icon) {
    entry.icon->mark_deleted();
    entry.icon = nullptr;
  }
  if (entry.score_text_entity) {
    entry.score_text_entity->mark_deleted();
    entry.score_text_entity = nullptr;
  }
  entry.icon_transform = nullptr;
  entry.score_text_transform = nullptr;
  entry.score_text = nullptr;
}

inline void clear_entries() {
  for (auto& entry : entries) {
    destroy_entry(entry);
  }
  entries.clear();
  entry_index.clear();
  active_rows = 0;
}

inline void destroy_score_visual() {
  if (score_text_entity) {
    score_text_entity->mark_deleted();
  }
  score_text_entity = nullptr;
  score_text_transform = nullptr;
  score_text = nullptr;
}

inline void destroy_heart_visuals() {
  for (size_t i = 0; i < kMaxHeartIcons; ++i) {
    if (heart_entities[i]) {
      heart_entities[i]->mark_deleted();
    }
    heart_entities[i] = nullptr;
    heart_transforms[i] = nullptr;
    heart_sprites[i] = nullptr;
    heart_colors[i] = nullptr;
  }
}

inline void reset_panel() {
  if (panel) {
    panel->mark_deleted();
  }
  panel = arena::create<ecs::Entity>();
  panel_transform = arena::create<transform::NoRotationTransform>();
  const glm::vec2 size = panel_size_px();
  panel_transform->pos = shrooms::screen::center_to_top_left(current_panel_center_px(), size);
  panel->add(panel_transform);
  panel->add(arena::create<layers::ConstLayer>(config.layer));
  const engine::TextureId tex_id = engine::resources::register_texture("menu_scoreboard");
  panel->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
  panel->add(arena::create<scene::SceneObject>("main"));
}

inline void hide_panel() {
  if (panel) {
    panel->mark_deleted();
  }
  panel = nullptr;
  panel_transform = nullptr;
  destroy_score_visual();
  destroy_heart_visuals();
}

inline void ensure_panel() {
  if (!panel) {
    reset_panel();
  }
}

inline void ensure_score_visual() {
  if (score_text_entity) return;

  score_text_entity = arena::create<ecs::Entity>();
  score_text_transform = arena::create<transform::NoRotationTransform>();
  score_text_entity->add(score_text_transform);
  score_text_entity->add(arena::create<layers::ConstLayer>(config.layer + 1));
  score_text = arena::create<text::TextObject>("0", config.score_font_px);
  score_text_entity->add(score_text);
  score_text_entity->add(arena::create<color::OneColor>(config.text_color));
  score_text_entity->add(arena::create<scene::SceneObject>("main"));
  score_text_entity->bind();
}

inline void ensure_heart_visuals() {
  const engine::TextureId tex_id = engine::resources::register_texture("heart");
  for (size_t i = 0; i < kMaxHeartIcons; ++i) {
    if (heart_entities[i]) continue;

    heart_entities[i] = arena::create<ecs::Entity>();
    heart_transforms[i] = arena::create<transform::NoRotationTransform>();
    heart_entities[i]->add(heart_transforms[i]);
    heart_entities[i]->add(arena::create<layers::ConstLayer>(config.layer + 1));
    const glm::vec2 size = heart_size_px();
    heart_sprites[i] = arena::create<render_system::SpriteRenderable>(tex_id, size);
    heart_entities[i]->add(heart_sprites[i]);
    heart_colors[i] = arena::create<color::OneColor>(glm::vec4{1.0f});
    heart_entities[i]->add(heart_colors[i]);
    heart_entities[i]->add(arena::create<scene::SceneObject>("main"));
    heart_entities[i]->bind();
  }
}

inline ecs::Entity* create_sprite(const std::string& texture_name, const glm::vec2& center,
                                  const glm::vec2& size, int layer) {
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = shrooms::screen::center_to_top_left(center, size);
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(layer));
  const engine::TextureId tex_id = engine::resources::register_texture(texture_name);
  entity->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
  entity->add(arena::create<scene::SceneObject>("main"));
  return entity;
}

inline void update_score_layout() {
  if (!status_panel_visible) return;
  ensure_score_visual();
  if (!score_text || !score_text_transform) return;

  const std::string value = std::to_string(current_score);
  score_text->text = value;
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, config.score_font_px);
  const glm::vec2 size{layout.width, layout.height};
  score_text_transform->pos = score_anchor_px() - size * 0.5f;
}

inline void update_heart_layout() {
  if (!hearts_visible) {
    destroy_heart_visuals();
    return;
  }

  ensure_heart_visuals();
  const glm::vec2 heart_size = heart_size_px();
  const float gap = heart_gap_px(heart_size);
  const float row_width = heart_row_width_px(heart_size, gap);
  const glm::vec2 row_center = layout_row_center_px(1);
  const glm::vec2 row_start = row_center - glm::vec2{row_width * 0.5f, 0.0f};
  const int filled_hearts = std::clamp(current_hearts, 0, static_cast<int>(kMaxHeartIcons));

  for (size_t i = 0; i < kMaxHeartIcons; ++i) {
    const glm::vec2 center =
        row_start + glm::vec2{heart_size.x * 0.5f +
                                  static_cast<float>(i) * (heart_size.x + gap),
                              0.0f};
    if (heart_transforms[i]) {
      heart_transforms[i]->pos = shrooms::screen::center_to_top_left(center, heart_size);
    }
    if (heart_sprites[i]) {
      heart_sprites[i]->size = heart_size;
      heart_sprites[i]->geometry = engine::geometry::make_quad(heart_size.x, heart_size.y);
      heart_sprites[i]->uploaded = false;
    }
    if (heart_colors[i]) {
      const bool filled = static_cast<int>(i) < filled_hearts;
      heart_colors[i]->color =
          filled ? glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}
                 : glm::vec4{0.34f, 0.34f, 0.38f, 0.82f};
    }
  }
}

inline void update_entry_layout(Entry& entry) {
  if (!entry.score_text || !entry.score_text_transform) return;

  const std::string value = recipe_score_text_value(entry);
  entry.score_text->text = value;
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, config.text_font_px);
  const glm::vec2 text_size{layout.width, layout.height};
  const auto* sprite = entry.icon ? entry.icon->get<render_system::SpriteRenderable>() : nullptr;
  const glm::vec2 icon_size = sprite ? sprite->size : glm::vec2{0.0f, 0.0f};
  const glm::vec2 row_center = layout_row_center_px(1 + entry.row_index);
  const float total_width = icon_size.x + config.row_icon_gap_px + text_size.x;
  const float row_left = row_center.x - total_width * 0.5f;

  if (entry.icon_transform && sprite) {
    const glm::vec2 icon_center{row_left + icon_size.x * 0.5f, row_center.y};
    entry.icon_transform->pos = shrooms::screen::center_to_top_left(icon_center, icon_size);
  }
  entry.score_text_transform->pos =
      glm::vec2{row_left + icon_size.x + config.row_icon_gap_px,
                row_center.y - text_size.y * 0.5f};
}

inline void apply_layout() {
  if (!status_panel_visible) return;
  ensure_panel();
  active_rows = layout_row_count();

  if (panel_transform) {
    const glm::vec2 size = panel_size_px();
    panel_transform->pos = shrooms::screen::center_to_top_left(current_panel_center_px(), size);
  }

  update_score_layout();
  update_heart_layout();
  for (auto& entry : entries) {
    update_entry_layout(entry);
  }
}

inline void create_entry_visuals(Entry& entry, size_t index) {
  const std::string icon_name = icon_texture_name(entry.name);
  const float icon_width =
      shrooms::screen::scale_to_pixels(glm::vec2{config.icon_scale.x, 0.0f}).x;
  const glm::vec2 icon_size =
      shrooms::texture_sizing::from_width_px(icon_name, icon_width);
  const glm::vec2 icon_center = layout_row_center_px(1 + index);
  entry.icon = create_sprite(icon_name, icon_center, icon_size, config.layer + 1);
  entry.icon_transform = entry.icon ? entry.icon->get<transform::NoRotationTransform>() : nullptr;
  entry.icon->bind();

  entry.row_index = index;
  entry.score_text_entity = arena::create<ecs::Entity>();
  entry.score_text_transform = arena::create<transform::NoRotationTransform>();
  entry.score_text_entity->add(entry.score_text_transform);
  entry.score_text_entity->add(arena::create<layers::ConstLayer>(config.layer + 1));
  entry.score_text = arena::create<text::TextObject>("", config.text_font_px);
  entry.score_text_entity->add(entry.score_text);
  entry.score_text_entity->add(arena::create<color::OneColor>(config.text_color));
  entry.score_text_entity->add(arena::create<scene::SceneObject>("main"));
  entry.score_text_entity->bind();

  update_entry_layout(entry);
}

inline void rebuild_entries() {
  clear_entries();
  entries.reserve(current_recipe.size());
  for (size_t i = 0; i < current_recipe.size(); ++i) {
    Entry entry{};
    entry.name = current_recipe[i].first;
    entry.target = current_recipe[i].second;
    entry.current = 0;
    create_entry_visuals(entry, i);
    entry_index[entry.name] = entries.size();
    entries.push_back(entry);
  }
}

inline void init_with_targets(const std::vector<std::pair<std::string, int>>& recipe,
                              std::string task_word = "") {
  objective_word = std::move(task_word);
  current_recipe = recipe;
  status_panel_visible = true;
  layout_state = LayoutState::Corner;
  current_panel_center_norm = target_center_norm(layout_state);
  ensure_panel();
  rebuild_entries();
  apply_layout();
}

inline void hide() {
  status_panel_visible = false;
  animation_active = false;
  animation_elapsed = 0.0f;
  animation_duration = 0.0f;
  shake_duration = 0.0f;
  shake_offset_px = glm::vec2{0.0f, 0.0f};
  hearts_visible = false;
  current_recipe.clear();
  objective_word.clear();
  clear_entries();
  hide_panel();
}

inline void set_score(int score_value) {
  current_score = score_value;
  update_score_layout();
}

inline void set_hearts(int hearts_value) {
  current_hearts = std::clamp(hearts_value, 0, static_cast<int>(kMaxHeartIcons));
  update_heart_layout();
}

inline void set_hearts_visible(bool visible) {
  hearts_visible = visible;
  apply_layout();
}

inline void update_score(const std::string& name, int new_score, int target) {
  auto it = entry_index.find(name);
  if (it == entry_index.end()) return;
  auto& entry = entries[it->second];
  entry.current = new_score;
  entry.target = target;
  update_entry_layout(entry);
}

inline void set_layout(LayoutState state) {
  if (state == LayoutState::Hidden) {
    layout_state = state;
    hide();
    return;
  }

  status_panel_visible = true;
  layout_state = state;
  animation_active = false;
  animation_elapsed = 0.0f;
  animation_duration = 0.0f;
  current_panel_center_norm = target_center_norm(state);
  apply_layout();
}

inline void animate_to_layout(LayoutState state, float duration = config.default_move_duration) {
  if (state == LayoutState::Hidden) {
    set_layout(state);
    return;
  }

  status_panel_visible = true;
  layout_state = state;
  animation_start_norm = panel_center_norm();
  animation_target_norm = target_center_norm(state);
  animation_elapsed = 0.0f;
  animation_duration = std::max(0.0f, duration);
  animation_active = animation_duration > 0.0f;
  if (!animation_active) {
    current_panel_center_norm = animation_target_norm;
  }
  apply_layout();
}

inline void start_intro_move_to_corner(float duration = 2.3f) {
  if (current_recipe.empty()) return;
  set_layout(LayoutState::CenterIntro);
  animate_to_layout(LayoutState::Corner, duration);
}

inline void start_shake(float duration = config.shake_duration,
                        float amplitude_px = config.shake_amplitude_px) {
  if (current_recipe.empty()) return;
  animation_active = false;
  animation_elapsed = 0.0f;
  animation_duration = 0.0f;
  shake_elapsed = 0.0f;
  shake_duration = std::max(0.0f, duration);
  active_shake_amplitude_px = amplitude_px;
  shake_offset_px = glm::vec2{0.0f, 0.0f};
  apply_layout();
}

inline void start_center_shake(float duration = config.shake_duration,
                               float amplitude_px = config.shake_amplitude_px) {
  if (current_recipe.empty()) return;
  set_layout(LayoutState::CenterIntro);
  start_shake(duration, amplitude_px);
}

inline bool is_animating() {
  return animation_active || shake_duration > 0.0f;
}

struct ScoreboardController : public dynamic::DynamicObject {
  ScoreboardController() : dynamic::DynamicObject() {}
  ~ScoreboardController() override { Component::component_count--; }

  void update() override {
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    bool dirty = false;

    if (animation_active) {
      animation_elapsed += dt;
      const float t =
          animation_duration > 0.0f ? clamp01(animation_elapsed / animation_duration) : 1.0f;
      const float eased = ease_in_out(t);
      current_panel_center_norm =
          animation_start_norm + (animation_target_norm - animation_start_norm) * eased;
      if (t >= 1.0f) {
        animation_active = false;
        current_panel_center_norm = animation_target_norm;
      }
      dirty = true;
    }

    if (shake_duration > 0.0f) {
      shake_elapsed += dt;
      const float t = clamp01(shake_elapsed / shake_duration);
      const float strength = 1.0f - t;
      const float time = static_cast<float>(ecs::context().time_seconds);
      shake_offset_px = glm::vec2{
          std::sin(time * config.shake_frequency) * active_shake_amplitude_px * strength,
          std::sin(time * config.shake_frequency * 1.37f) * active_shake_amplitude_px * 0.45f *
              strength,
      };
      if (t >= 1.0f) {
        shake_duration = 0.0f;
        shake_offset_px = glm::vec2{0.0f, 0.0f};
      }
      dirty = true;
    }

    if (dirty) {
      apply_layout();
    }
  }
};

inline ScoreboardController controller{};

inline void init() {
  hide();
  current_score = 0;
  current_hearts = 0;
  hearts_visible = false;
  layout_state = LayoutState::Corner;
  current_panel_center_norm = corner_panel_center_norm();
  animation_active = false;
  shake_duration = 0.0f;
  active_shake_amplitude_px = config.shake_amplitude_px;
  shake_offset_px = glm::vec2{0.0f, 0.0f};
}

}  // namespace scoreboard
