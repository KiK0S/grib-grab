#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <utility>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/color/color_system.hpp"
#include "systems/hidden/hidden_object.hpp"
#include "systems/input/input_system.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/render_system.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"

#include "camera_shake.hpp"
#include "countdown.hpp"
#include "game_audio.hpp"
#include "level_manager.hpp"
#include "level_intro.hpp"
#include "score_hud.hpp"
#include "menu.hpp"
#include "player.hpp"
#include "round_transition.hpp"
#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"
#include "vfx.hpp"

namespace pause_menu {

constexpr int kKeyArrowUpDom = 38;
constexpr int kKeyArrowDownDom = 40;
constexpr int kKeyArrowLeftDom = 37;
constexpr int kKeyArrowRightDom = 39;
constexpr int kKeyArrowUpSdl = 1073741906;
constexpr int kKeyArrowDownSdl = 1073741905;
constexpr int kKeyArrowLeftSdl = 1073741904;
constexpr int kKeyArrowRightSdl = 1073741903;
constexpr const char* kPauseToggleIconTexture = "in_game_pause";
constexpr float kVolumeAdjustStep = 0.05f;

struct ActionTextColor : public color::ColoredObject {
  explicit ActionTextColor(const glm::vec4& value) : color::ColoredObject(), value(value) {}
  ~ActionTextColor() override { Component::component_count--; }
  glm::vec4 get_color() override { return value; }
  glm::vec4 value{1.0f, 1.0f, 1.0f, 1.0f};
};

struct Config {
  glm::vec2 overlay_scale = glm::vec2(1.0f, 1.0f);
  glm::vec2 menu_scale = glm::vec2(0.15f, 0.15f);
  glm::vec2 menu_position = glm::vec2(0.0f, 0.0f);
  glm::vec4 overlay_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  float overlay_top_alpha = 0.50f;
  float overlay_bottom_alpha = 0.10f;
  glm::vec4 menu_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  glm::vec2 resume_scale = glm::vec2(0.25f, 0.08f);
  glm::vec2 resume_offset = glm::vec2(0.0f, -0.03f);
  glm::vec2 restart_scale = glm::vec2(0.25f, 0.08f);
  glm::vec2 restart_offset = glm::vec2(0.0f, -0.17f);
  glm::vec2 audio_scale = glm::vec2(0.25f, 0.12f);
  glm::vec2 audio_offset = glm::vec2(0.0f, -0.31f);
  glm::vec2 main_menu_scale = glm::vec2(0.25f, 0.08f);
  glm::vec2 main_menu_offset = glm::vec2(0.0f, -0.45f);
  glm::vec2 pause_button_position = glm::vec2(0.90f, -0.90f);
  glm::vec2 pause_button_scale = glm::vec2(0.095f, 0.095f);
  glm::vec2 pause_icon_scale = glm::vec2(0.055f, 0.055f);
  float countdown_scale = 0.15f;
  int overlay_layer = 100;
  int menu_layer = 101;
  int button_layer = 102;
  int text_layer = 103;
  int pause_button_layer = 104;
} config;

struct ActionLine {
  ecs::Entity* button_entity = nullptr;
  transform::NoRotationTransform* button_transform = nullptr;
  render_system::QuadRenderable* button_quad = nullptr;
  hidden::HiddenObject* button_hidden = nullptr;

  ecs::Entity* text_entity = nullptr;
  transform::NoRotationTransform* text_transform = nullptr;
  text::TextObject* text_object = nullptr;
  hidden::HiddenObject* text_hidden = nullptr;
  ActionTextColor* text_color = nullptr;
  ecs::Entity* slider_track_entity = nullptr;
  transform::NoRotationTransform* slider_track_transform = nullptr;
  render_system::QuadRenderable* slider_track_quad = nullptr;
  hidden::HiddenObject* slider_track_hidden = nullptr;
  ecs::Entity* slider_fill_entity = nullptr;
  transform::NoRotationTransform* slider_fill_transform = nullptr;
  render_system::QuadRenderable* slider_fill_quad = nullptr;
  hidden::HiddenObject* slider_fill_hidden = nullptr;
  ecs::Entity* slider_knob_entity = nullptr;
  transform::NoRotationTransform* slider_knob_transform = nullptr;
  render_system::QuadRenderable* slider_knob_quad = nullptr;
  hidden::HiddenObject* slider_knob_hidden = nullptr;

  glm::vec2 button_base_pos{0.0f, 0.0f};
  glm::vec2 button_base_size{0.0f, 0.0f};
  glm::vec2 slider_track_pos{0.0f, 0.0f};
  glm::vec2 slider_track_size{0.0f, 0.0f};
  float slider_value = 1.0f;
  engine::UIColor base_color{0.15f, 0.15f, 0.15f, 0.95f};
  engine::UIColor hover_color{0.28f, 0.2f, 0.35f, 0.98f};
  engine::UIColor selected_color{0.15f, 0.15f, 0.15f, 0.95f};
  engine::UIColor dimmed_color{0.08f, 0.08f, 0.08f, 0.52f};
  engine::UIColor slider_track_color{0.92f, 0.94f, 0.98f, 1.0f};
  engine::UIColor slider_fill_color{0.18f, 0.78f, 1.0f, 1.0f};
  engine::UIColor slider_knob_color{1.0f, 1.0f, 1.0f, 1.0f};
  engine::UIColor slider_dimmed_color{0.72f, 0.74f, 0.8f, 1.0f};
  glm::vec4 base_text_color{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 selected_text_color{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 dimmed_text_color{0.72f, 0.72f, 0.72f, 0.92f};
  float hover_scale = 1.05f;
  float selected_scale = 1.0f;
  float font_px = 24.0f;
};

inline constexpr size_t kResumeAction = 0;
inline constexpr size_t kRestartAction = 1;
inline constexpr size_t kAudioAction = 2;
inline constexpr size_t kMainMenuAction = 3;
inline constexpr size_t kActionCount = 4;

inline ecs::Entity* overlay = nullptr;
inline render_system::QuadRenderable* overlay_quad = nullptr;
inline ecs::Entity* menu_window = nullptr;
inline std::array<ActionLine, kActionCount> action_lines{};

inline ecs::Entity* pause_toggle_button = nullptr;
inline transform::NoRotationTransform* pause_toggle_transform = nullptr;
inline render_system::QuadRenderable* pause_toggle_quad = nullptr;
inline hidden::HiddenObject* pause_toggle_button_hidden = nullptr;
inline glm::vec2 pause_toggle_base_pos{0.0f, 0.0f};
inline glm::vec2 pause_toggle_base_size{0.0f, 0.0f};

inline ecs::Entity* pause_toggle_icon = nullptr;
inline render_system::SpriteRenderable* pause_toggle_icon_sprite = nullptr;
inline hidden::HiddenObject* pause_toggle_icon_hidden = nullptr;

inline ecs::Entity* pause_menu_icon = nullptr;
inline hidden::HiddenObject* pause_menu_icon_hidden = nullptr;

inline hidden::HiddenObject* overlay_hidden = nullptr;
inline hidden::HiddenObject* menu_hidden = nullptr;
inline bool pause_menu_open = false;

inline engine::UIColor pause_toggle_base_color{0.15f, 0.15f, 0.15f, 0.92f};
inline engine::UIColor pause_toggle_hover_color{0.22f, 0.22f, 0.22f, 0.98f};
inline float pause_toggle_hover_scale = 1.05f;

inline bool is_arrow_up_key(int key) {
  return key == kKeyArrowUpDom || key == kKeyArrowUpSdl;
}

inline bool is_arrow_down_key(int key) {
  return key == kKeyArrowDownDom || key == kKeyArrowDownSdl;
}

inline bool is_arrow_left_key(int key) {
  return key == kKeyArrowLeftDom || key == kKeyArrowLeftSdl;
}

inline bool is_arrow_right_key(int key) {
  return key == kKeyArrowRightDom || key == kKeyArrowRightSdl;
}

inline bool is_confirm_key(int key) {
  return key == ' ' || key == '\r' || key == '\n' || key == 13;
}

inline bool point_in_bounds(const glm::vec2& point, const glm::vec2& min_bound,
                            const glm::vec2& max_bound) {
  if (point.x < min_bound.x || point.x > max_bound.x) return false;
  if (point.y < min_bound.y || point.y > max_bound.y) return false;
  return true;
}

inline bool is_main_scene_active() {
  auto* active = scene::get_active_scene();
  return active && active->get_name() == "main";
}

inline bool pause_controls_blocked() {
  return levels::has_pending_failure() || levels::level_finished || round_transition::is_active() ||
         level_intro::is_active();
}

inline float clamp_unit(float value) {
  return std::clamp(value, 0.0f, 1.0f);
}

inline void set_quad_size(render_system::QuadRenderable* quad, float width, float height) {
  if (!quad) return;
  quad->width = width;
  quad->height = height;
  quad->geometry = engine::geometry::make_quad(width, height);
  quad->uploaded = false;
}

inline void set_vertex_color(engine::Vertex& vertex, const glm::vec4& value) {
  vertex.r = value.x;
  vertex.g = value.y;
  vertex.b = value.z;
  vertex.a = value.w;
}

inline engine::GeometryData make_overlay_gradient_geometry(float width, float height) {
  engine::GeometryData geometry = engine::geometry::make_quad(width, height);
  if (geometry.vertices.size() < 4) return geometry;

  const glm::vec4 top_color{config.overlay_color.x, config.overlay_color.y,
                            config.overlay_color.z, clamp_unit(config.overlay_top_alpha)};
  const glm::vec4 bottom_color{config.overlay_color.x, config.overlay_color.y,
                               config.overlay_color.z,
                               clamp_unit(config.overlay_bottom_alpha)};
  set_vertex_color(geometry.vertices[0], top_color);
  set_vertex_color(geometry.vertices[1], top_color);
  set_vertex_color(geometry.vertices[2], bottom_color);
  set_vertex_color(geometry.vertices[3], bottom_color);
  return geometry;
}

inline void update_action_slider_geometry(ActionLine& action) {
  if (!action.slider_track_quad || !action.slider_fill_quad || !action.slider_knob_quad ||
      !action.slider_track_transform || !action.slider_fill_transform || !action.slider_knob_transform ||
      !action.button_transform || !action.button_quad) {
    return;
  }

  const glm::vec2 button_pos = action.button_transform->pos;
  const glm::vec2 button_size{action.button_quad->width, action.button_quad->height};
  if (button_size.x <= 0.0f || button_size.y <= 0.0f) return;

  const float pad_x = std::clamp(button_size.x * 0.08f, 14.0f, 24.0f);
  const float top_pad = std::max(6.0f, button_size.y * 0.10f);
  const float bottom_pad = std::max(6.0f, button_size.y * 0.10f);
  const float inner_h = std::max(1.0f, button_size.y - top_pad - bottom_pad);
  const float text_band_h = inner_h * 0.60f;
  const float slider_band_h = std::max(1.0f, inner_h - text_band_h);
  const float track_w = std::max(72.0f, button_size.x - pad_x * 2.0f);
  const float track_h = std::clamp(slider_band_h * 0.62f, 12.0f, 20.0f);
  const float track_x = button_pos.x + pad_x;
  const float track_y = button_pos.y + top_pad + text_band_h + (slider_band_h - track_h) * 0.5f;
  const float knob_size = std::max(18.0f, slider_band_h * 1.05f);
  const float t = clamp_unit(action.slider_value);

  action.slider_track_pos = glm::vec2{track_x, track_y};
  action.slider_track_size = glm::vec2{track_w, track_h};

  action.slider_track_transform->pos = action.slider_track_pos;
  set_quad_size(action.slider_track_quad, track_w, track_h);

  action.slider_fill_transform->pos = action.slider_track_pos;
  set_quad_size(action.slider_fill_quad, track_w * t, track_h);

  const float knob_center_x = track_x + track_w * t;
  const float knob_center_y = track_y + track_h * 0.5f;
  action.slider_knob_transform->pos =
      glm::vec2{knob_center_x - knob_size * 0.5f, knob_center_y - knob_size * 0.5f};
  set_quad_size(action.slider_knob_quad, knob_size, knob_size);
}

inline void ensure_action_slider(ActionLine& action) {
  if (action.slider_track_entity || !action.button_entity) return;

  const int slider_layer = config.button_layer + 1;
  action.slider_track_entity = arena::create<ecs::Entity>();
  action.slider_track_transform = arena::create<transform::NoRotationTransform>();
  action.slider_track_entity->add(action.slider_track_transform);
  action.slider_track_entity->add(arena::create<layers::ConstLayer>(slider_layer));
  action.slider_track_quad = arena::create<render_system::QuadRenderable>(0.0f, 0.0f, action.slider_track_color);
  action.slider_track_entity->add(action.slider_track_quad);
  action.slider_track_hidden = arena::create<hidden::HiddenObject>();
  action.slider_track_entity->add(action.slider_track_hidden);
  action.slider_track_hidden->hide();
  action.slider_track_entity->add(arena::create<scene::SceneObject>("main"));

  action.slider_fill_entity = arena::create<ecs::Entity>();
  action.slider_fill_transform = arena::create<transform::NoRotationTransform>();
  action.slider_fill_entity->add(action.slider_fill_transform);
  action.slider_fill_entity->add(arena::create<layers::ConstLayer>(slider_layer));
  action.slider_fill_quad = arena::create<render_system::QuadRenderable>(0.0f, 0.0f, action.slider_fill_color);
  action.slider_fill_entity->add(action.slider_fill_quad);
  action.slider_fill_hidden = arena::create<hidden::HiddenObject>();
  action.slider_fill_entity->add(action.slider_fill_hidden);
  action.slider_fill_hidden->hide();
  action.slider_fill_entity->add(arena::create<scene::SceneObject>("main"));

  action.slider_knob_entity = arena::create<ecs::Entity>();
  action.slider_knob_transform = arena::create<transform::NoRotationTransform>();
  action.slider_knob_entity->add(action.slider_knob_transform);
  action.slider_knob_entity->add(arena::create<layers::ConstLayer>(slider_layer));
  action.slider_knob_quad = arena::create<render_system::QuadRenderable>(0.0f, 0.0f, action.slider_knob_color);
  action.slider_knob_entity->add(action.slider_knob_quad);
  action.slider_knob_hidden = arena::create<hidden::HiddenObject>();
  action.slider_knob_entity->add(action.slider_knob_hidden);
  action.slider_knob_hidden->hide();
  action.slider_knob_entity->add(arena::create<scene::SceneObject>("main"));

  update_action_slider_geometry(action);
}

inline void set_action_slider_value(ActionLine& action, float value) {
  action.slider_value = clamp_unit(value);
  update_action_slider_geometry(action);
}

inline void set_action_slider_visibility(ActionLine& action, bool visible) {
  if (action.slider_track_hidden) action.slider_track_hidden->set_visible(visible);
  if (action.slider_fill_hidden) action.slider_fill_hidden->set_visible(visible);
  if (action.slider_knob_hidden) action.slider_knob_hidden->set_visible(visible);
}

inline void apply_action_slider_visual(ActionLine& action, bool selected, bool hovered, bool dimmed) {
  if (!action.slider_track_quad || !action.slider_fill_quad || !action.slider_knob_quad) return;
  if (dimmed) {
    action.slider_track_quad->color = action.slider_dimmed_color;
    action.slider_fill_quad->color = action.slider_fill_color;
    action.slider_knob_quad->color = engine::UIColor{0.9f, 0.9f, 0.9f, 1.0f};
    return;
  }
  action.slider_track_quad->color = action.slider_track_color;
  action.slider_fill_quad->color = action.slider_fill_color;
  action.slider_knob_quad->color =
      (selected || hovered) ? engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f} : action.slider_knob_color;
}

inline bool point_hits_action_slider(const ActionLine& action, const glm::vec2& point) {
  if (!action.slider_track_quad) return false;
  const float extra_y = 10.0f;
  const glm::vec2 min_bound{action.slider_track_pos.x, action.slider_track_pos.y - extra_y};
  const glm::vec2 max_bound{action.slider_track_pos.x + action.slider_track_size.x,
                            action.slider_track_pos.y + action.slider_track_size.y + extra_y};
  return point_in_bounds(point, min_bound, max_bound);
}

inline void set_action_visibility(ActionLine& action, bool visible) {
  if (action.button_hidden) {
    action.button_hidden->set_visible(visible);
  }
  if (action.text_hidden) {
    action.text_hidden->set_visible(visible);
  }
  set_action_slider_visibility(action, visible && action.slider_track_entity);
}

inline void set_pause_menu_visible(bool visible) {
  pause_menu_open = visible;
  if (menu_hidden) menu_hidden->hide();
  if (pause_menu_icon_hidden) pause_menu_icon_hidden->set_visible(visible);
  for (auto& action : action_lines) {
    set_action_visibility(action, visible);
  }
}

inline void set_pause_toggle_visible(bool visible) {
  if (pause_toggle_button_hidden) {
    pause_toggle_button_hidden->set_visible(visible);
  }
  if (pause_toggle_icon_hidden) {
    pause_toggle_icon_hidden->set_visible(visible);
  }
}

inline void apply_action_visual(ActionLine& action, float scale, const engine::UIColor& color) {
  if (!action.button_transform || !action.button_quad) return;
  const glm::vec2 size = action.button_base_size * scale;
  const glm::vec2 center = action.button_base_pos + action.button_base_size * 0.5f;
  action.button_transform->pos = center - size * 0.5f;
  set_quad_size(action.button_quad, size.x, size.y);
  action.button_quad->color = color;
  update_action_slider_geometry(action);
}

inline void set_action_visual_state(ActionLine& action, bool selected, bool hovered, bool dimmed) {
  if (action.text_color) {
    action.text_color->value = selected ? action.selected_text_color
                                        : (dimmed ? action.dimmed_text_color
                                                  : action.base_text_color);
  }
  apply_action_slider_visual(action, selected, hovered, dimmed);
  if (selected) {
    apply_action_visual(action, action.selected_scale, action.selected_color);
    return;
  }
  if (hovered) {
    apply_action_visual(action, action.hover_scale, action.hover_color);
    return;
  }
  if (dimmed) {
    apply_action_visual(action, 1.0f, action.dimmed_color);
    return;
  }
  apply_action_visual(action, 1.0f, action.base_color);
}

inline std::pair<glm::vec2, glm::vec2> action_bounds(const ActionLine& action) {
  const glm::vec2 min_bound = action.button_base_pos;
  const glm::vec2 max_bound = action.button_base_pos + action.button_base_size;
  return {min_bound, max_bound};
}

inline std::pair<glm::vec2, glm::vec2> pause_toggle_bounds() {
  const glm::vec2 min_bound = pause_toggle_base_pos;
  const glm::vec2 max_bound = pause_toggle_base_pos + pause_toggle_base_size;
  return {min_bound, max_bound};
}

inline void apply_pause_toggle_hover(bool hovered) {
  if (pause_toggle_transform && pause_toggle_quad) {
    const float scale = hovered ? pause_toggle_hover_scale : 1.0f;
    const glm::vec2 size = pause_toggle_base_size * scale;
    const glm::vec2 center = pause_toggle_base_pos + pause_toggle_base_size * 0.5f;
    pause_toggle_transform->pos = center - size * 0.5f;
    set_quad_size(pause_toggle_quad, size.x, size.y);
    pause_toggle_quad->color = hovered ? pause_toggle_hover_color : pause_toggle_base_color;
  }
  if (pause_toggle_icon_sprite) {
    pause_toggle_icon_sprite->tint =
        hovered ? engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f}
                : engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f};
  }
}

inline void update_action_label(ActionLine& action, const std::string& label) {
  if (!action.text_object || !action.text_transform) return;
  action.text_object->text = label;
  const auto layout = engine::text::layout_text(label, 0.0f, 0.0f, action.font_px);
  const glm::vec2 text_size{layout.width, layout.height};
  const float text_y_centered = action.button_base_pos.y + (action.button_base_size.y - text_size.y) * 0.5f;
  if (action.slider_track_entity && action.slider_track_size.x > 0.0f) {
    const float top_pad = std::max(6.0f, action.button_base_size.y * 0.10f);
    const float bottom_pad = std::max(6.0f, action.button_base_size.y * 0.10f);
    const float inner_h = std::max(1.0f, action.button_base_size.y - top_pad - bottom_pad);
    const float text_band_h = inner_h * 0.60f;
    const float text_y = action.button_base_pos.y + top_pad + (text_band_h - text_size.y) * 0.5f;
    action.text_transform->pos = glm::vec2{
        action.button_base_pos.x + (action.button_base_size.x - text_size.x) * 0.5f,
        text_y,
    };
  } else {
    action.text_transform->pos = glm::vec2{
        action.button_base_pos.x + (action.button_base_size.x - text_size.x) * 0.5f,
        text_y_centered,
    };
  }
  update_action_slider_geometry(action);
}

inline void refresh_audio_action_label() {
  update_action_label(action_lines[kAudioAction], shrooms::audio::volume_label());
  set_action_slider_value(action_lines[kAudioAction], shrooms::audio::volume_slider_value());
}

inline ActionLine make_action_line(const std::string& label, const glm::vec2& center_norm,
                                   const glm::vec2& button_scale) {
  ActionLine action{};
  const glm::vec2 center = shrooms::screen::norm_to_pixels(center_norm);
  const glm::vec2 button_size = shrooms::screen::scale_to_pixels(button_scale);

  action.button_entity = arena::create<ecs::Entity>();
  action.button_transform = arena::create<transform::NoRotationTransform>();
  action.button_transform->pos = shrooms::screen::center_to_top_left(center, button_size);
  action.button_base_pos = action.button_transform->pos;
  action.button_base_size = button_size;
  action.button_entity->add(action.button_transform);
  action.button_entity->add(arena::create<layers::ConstLayer>(config.button_layer));
  action.button_quad = arena::create<render_system::QuadRenderable>(
      button_size.x, button_size.y, action.base_color);
  action.button_entity->add(action.button_quad);
  action.button_hidden = arena::create<hidden::HiddenObject>();
  action.button_entity->add(action.button_hidden);
  action.button_hidden->hide();
  action.button_entity->add(arena::create<scene::SceneObject>("main"));

  action.text_entity = arena::create<ecs::Entity>();
  action.text_transform = arena::create<transform::NoRotationTransform>();
  const float font_px = action.font_px;
  const auto layout = engine::text::layout_text(label, 0.0f, 0.0f, font_px);
  const glm::vec2 text_size{layout.width, layout.height};
  action.text_transform->pos = shrooms::screen::center_to_top_left(center, text_size);
  action.text_entity->add(action.text_transform);
  action.text_entity->add(arena::create<layers::ConstLayer>(config.text_layer + 1));
  action.text_object = arena::create<text::TextObject>(label, font_px);
  action.text_entity->add(action.text_object);
  action.text_color = arena::create<ActionTextColor>(action.base_text_color);
  action.text_entity->add(action.text_color);
  action.text_hidden = arena::create<hidden::HiddenObject>();
  action.text_entity->add(action.text_hidden);
  action.text_hidden->hide();
  action.text_entity->add(arena::create<scene::SceneObject>("main"));

  return action;
}

inline void relayout_actions_for_variable_heights() {
  constexpr std::array<size_t, kActionCount> order{
      kResumeAction, kRestartAction, kAudioAction, kMainMenuAction};
  const float gap_px = 12.0f;

  float next_top = action_lines[order[0]].button_base_pos.y;
  for (size_t i = 0; i < order.size(); ++i) {
    auto& action = action_lines[order[i]];
    if (i > 0 && action.button_base_pos.y < next_top) {
      action.button_base_pos.y = next_top;
      if (action.button_transform) {
        action.button_transform->pos = action.button_base_pos;
      }
    }
    if (action.text_object) {
      update_action_label(action, action.text_object->text);
    }
    next_top = action.button_base_pos.y + action.button_base_size.y + gap_px;
  }
}

struct PauseMenuController : public dynamic::DynamicObject {
  PauseMenuController() : dynamic::DynamicObject() {}
  ~PauseMenuController() override { Component::component_count--; }

  void update_pointer() {
    for (const auto& evt : input::events()) {
      if (evt.kind == engine::InputKind::PointerMove ||
          evt.kind == engine::InputKind::PointerDown) {
        last_pointer = glm::vec2{static_cast<float>(evt.x), static_cast<float>(evt.y)};
      }
    }
  }

  void move_selected_action(int direction) {
    if (direction == 0) return;
    const int count = static_cast<int>(kActionCount);
    int next = static_cast<int>(selected_action_index) + (direction > 0 ? 1 : -1);
    while (next < 0) {
      next += count;
    }
    next %= count;
    selected_action_index = static_cast<size_t>(next);
  }

  void apply_action_visuals(std::optional<size_t> hovered_index) {
    for (size_t i = 0; i < action_lines.size(); ++i) {
      const bool selected = (i == selected_action_index);
      const bool hovered = hovered_index && *hovered_index == i;
      set_action_visual_state(action_lines[i], selected, hovered, !selected);
    }
  }

  void trigger_action(size_t index) {
    if (index == kRestartAction) {
      handle_restart();
      return;
    }
    if (index == kAudioAction) {
      handle_audio_toggle();
      return;
    }
    if (index == kMainMenuAction) {
      handle_main_menu();
      return;
    }
    handle_toggle();
  }

  void change_volume(float delta) {
    shrooms::audio::set_master_gain(shrooms::audio::master_gain() + delta);
    cached_master_gain = shrooms::audio::master_gain();
    refresh_audio_action_label();
    apply_action_visuals(std::nullopt);
  }

  void set_volume_from_pointer(const glm::vec2& point) {
    auto& audio_action = action_lines[kAudioAction];
    if (audio_action.slider_track_size.x <= 0.0f) return;
    const float t =
        clamp_unit((point.x - audio_action.slider_track_pos.x) / audio_action.slider_track_size.x);
    shrooms::audio::set_master_gain(t);
    cached_master_gain = shrooms::audio::master_gain();
    refresh_audio_action_label();
    apply_action_visuals(std::nullopt);
  }

  void update() override {
    if (cached_audio_muted != shrooms::audio::is_muted()) {
      cached_audio_muted = shrooms::audio::is_muted();
      refresh_audio_action_label();
    }
    if (cached_master_gain != shrooms::audio::master_gain()) {
      cached_master_gain = shrooms::audio::master_gain();
      refresh_audio_action_label();
    }

    update_pointer();

    const bool main_active = is_main_scene_active();
    if (!main_active) {
      dragging_audio_slider = false;
      set_pause_toggle_visible(false);
      apply_pause_toggle_hover(false);
      set_pause_menu_visible(false);
      return;
    }

    const bool paused = scene::is_current_scene_paused();
    const bool blocked = pause_controls_blocked();

    for (const auto& evt : input::events()) {
      if (evt.kind != engine::InputKind::KeyDown) continue;
      const int key = input::normalize_key_code(evt.key_code);
      if (key != 'P') continue;
      if (paused || !blocked) {
        handle_toggle();
        return;
      }
    }

    if (!paused) {
      dragging_audio_slider = false;
      set_pause_menu_visible(false);
      const bool show_toggle = !blocked;
      set_pause_toggle_visible(show_toggle);
      if (!show_toggle) {
        apply_pause_toggle_hover(false);
        return;
      }

      bool hovered_toggle = false;
      if (last_pointer) {
        const auto [min_bound, max_bound] = pause_toggle_bounds();
        hovered_toggle = point_in_bounds(*last_pointer, min_bound, max_bound);
      }
      apply_pause_toggle_hover(hovered_toggle);

      for (const auto& evt : input::events()) {
        if (evt.kind != engine::InputKind::PointerDown) continue;
        if (!hovered_toggle) continue;
        handle_toggle();
        return;
      }
      return;
    }

    set_pause_toggle_visible(false);
    apply_pause_toggle_hover(false);
    if (!pause_menu_open) {
      dragging_audio_slider = false;
      return;
    }

    for (const auto& evt : input::events()) {
      if (evt.kind == engine::InputKind::PointerUp) {
        dragging_audio_slider = false;
      }
    }

    std::optional<size_t> hovered_index;
    if (last_pointer) {
      for (size_t i = 0; i < action_lines.size(); ++i) {
        const auto [min_bound, max_bound] = action_bounds(action_lines[i]);
        if (!point_in_bounds(*last_pointer, min_bound, max_bound)) continue;
        hovered_index = i;
        break;
      }
    }
    if (hovered_index) {
      selected_action_index = *hovered_index;
    }
    apply_action_visuals(hovered_index);

    if (dragging_audio_slider) {
      for (const auto& evt : input::events()) {
        if (evt.kind != engine::InputKind::PointerMove &&
            evt.kind != engine::InputKind::PointerDown) {
          continue;
        }
        set_volume_from_pointer(glm::vec2{static_cast<float>(evt.x), static_cast<float>(evt.y)});
        return;
      }
    }

    for (const auto& evt : input::events()) {
      if (evt.kind == engine::InputKind::PointerDown) {
        const glm::vec2 point{static_cast<float>(evt.x), static_cast<float>(evt.y)};
        if (point_hits_action_slider(action_lines[kAudioAction], point)) {
          selected_action_index = kAudioAction;
          dragging_audio_slider = true;
          set_volume_from_pointer(point);
          return;
        }
        if (hovered_index && *hovered_index == kAudioAction) {
          selected_action_index = kAudioAction;
          trigger_action(kAudioAction);
          return;
        }
        if (hovered_index) {
          trigger_action(*hovered_index);
          return;
        }
        continue;
      }
      if (evt.kind != engine::InputKind::KeyDown) continue;
      const int key = input::normalize_key_code(evt.key_code);
      if (is_arrow_up_key(key)) {
        move_selected_action(-1);
        apply_action_visuals(std::nullopt);
        continue;
      }
      if (is_arrow_down_key(key)) {
        move_selected_action(1);
        apply_action_visuals(std::nullopt);
        continue;
      }
      if (key == 'R') {
        trigger_action(kRestartAction);
        return;
      }
      if (key == 'V') {
        trigger_action(kAudioAction);
        return;
      }
      if (key == 'A' && selected_action_index == kAudioAction) {
        change_volume(-kVolumeAdjustStep);
        return;
      }
      if (key == 'D' && selected_action_index == kAudioAction) {
        change_volume(kVolumeAdjustStep);
        return;
      }
      if (is_arrow_left_key(key) && selected_action_index == kAudioAction) {
        change_volume(-kVolumeAdjustStep);
        return;
      }
      if (is_arrow_right_key(key) && selected_action_index == kAudioAction) {
        change_volume(kVolumeAdjustStep);
        return;
      }
      if (key == 'M') {
        trigger_action(kMainMenuAction);
        return;
      }
      if (key == 27) {
        trigger_action(kResumeAction);
        return;
      }
      if (is_confirm_key(key)) {
        trigger_action(selected_action_index);
        return;
      }
    }
  }

  void handle_toggle() {
    if (!scene::is_current_scene_paused()) {
      camera_shake::reset();
      vfx::reset_wobble_offsets();
      selected_action_index = kResumeAction;
      if (overlay_hidden) overlay_hidden->show();
      set_pause_menu_visible(true);
      apply_action_visuals(std::nullopt);
      scene::toggle_pause();
      return;
    }

    set_pause_menu_visible(false);
    countdown::start("main", 3, []() {
      camera_shake::reset();
      vfx::reset_wobble_offsets();
      if (overlay_hidden) overlay_hidden->hide();
      if (auto* main = scene::get_scene("main")) {
        main->set_pause(false);
      }
    });
  }

  void handle_restart() {
    levels::restart_level();
    score_hud::reset_for_run();
    player::reset_for_new_level();
    camera_shake::reset();
    vfx::reset_wobble_offsets();
    if (overlay_hidden) overlay_hidden->show();
    set_pause_menu_visible(false);
    if (auto* main = scene::get_scene("main")) {
      main->activate();
      main->set_pause(true);
    }
    countdown::start("main", 3, []() {
      camera_shake::reset();
      vfx::reset_wobble_offsets();
      if (overlay_hidden) overlay_hidden->hide();
      if (auto* main = scene::get_scene("main")) {
        main->set_pause(false);
      }
    });
  }

  void handle_main_menu() {
    camera_shake::reset();
    vfx::reset_wobble_offsets();
    countdown::cancel();
    set_pause_menu_visible(false);
    if (overlay_hidden) overlay_hidden->hide();
    if (auto* main = scene::get_scene("main")) {
      main->set_pause(true);
    }
    if (auto* menu_scene = scene::get_scene("menu")) {
      menu_scene->activate();
      menu_scene->set_pause(true);
    }
    menu::enter_main_menu_mode();
    menu::suppress_input_for_frames(2);
  }

  void handle_audio_toggle() {
    shrooms::audio::toggle_muted();
    cached_audio_muted = shrooms::audio::is_muted();
    refresh_audio_action_label();
    apply_action_visuals(std::nullopt);
  }

  std::optional<glm::vec2> last_pointer;
  size_t selected_action_index = kResumeAction;
  bool cached_audio_muted = shrooms::audio::is_muted();
  float cached_master_gain = shrooms::audio::master_gain();
  bool dragging_audio_slider = false;
};

inline PauseMenuController pause_menu_controller{};

inline void init() {
  countdown::config.scale = config.countdown_scale;
  const glm::vec2 view_size{
      static_cast<float>(shrooms::screen::view_width),
      static_cast<float>(shrooms::screen::view_height),
  };

  overlay = arena::create<ecs::Entity>();
  auto* overlay_transform = arena::create<transform::NoRotationTransform>();
  overlay_transform->pos = glm::vec2{0.0f, 0.0f};
  overlay->add(overlay_transform);
  overlay->add(arena::create<layers::ConstLayer>(config.overlay_layer));
  overlay_quad = arena::create<render_system::QuadRenderable>(
      view_size.x, view_size.y, engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f});
  overlay_quad->geometry = make_overlay_gradient_geometry(view_size.x, view_size.y);
  overlay_quad->uploaded = false;
  overlay->add(overlay_quad);
  overlay_hidden = arena::create<hidden::HiddenObject>();
  overlay->add(overlay_hidden);
  overlay_hidden->hide();
  overlay->add(arena::create<scene::SceneObject>("main"));

  menu_window = arena::create<ecs::Entity>();
  const glm::vec2 menu_size = shrooms::screen::scale_to_pixels(config.menu_scale);
  const glm::vec2 menu_center = shrooms::screen::norm_to_pixels(config.menu_position);
  auto* menu_transform = arena::create<transform::NoRotationTransform>();
  menu_transform->pos = shrooms::screen::center_to_top_left(menu_center, menu_size);
  menu_window->add(menu_transform);
  menu_window->add(arena::create<layers::ConstLayer>(config.menu_layer));
  const engine::TextureId menu_tex = engine::resources::register_texture("background");
  menu_window->add(arena::create<render_system::SpriteRenderable>(
      menu_tex, menu_size,
      engine::UIColor{config.menu_color.x, config.menu_color.y, config.menu_color.z,
                      config.menu_color.w}));
  menu_hidden = arena::create<hidden::HiddenObject>();
  menu_window->add(menu_hidden);
  menu_hidden->hide();
  menu_window->add(arena::create<scene::SceneObject>("main"));

  action_lines[kResumeAction] =
      make_action_line("Resume", config.menu_position + config.resume_offset, config.resume_scale);
  action_lines[kRestartAction] = make_action_line("Restart", config.menu_position + config.restart_offset,
                                                  config.restart_scale);
  action_lines[kAudioAction] =
      make_action_line(shrooms::audio::volume_label(),
                       config.menu_position + config.audio_offset, config.audio_scale);
  ensure_action_slider(action_lines[kAudioAction]);
  set_action_slider_value(action_lines[kAudioAction], shrooms::audio::volume_slider_value());
  action_lines[kMainMenuAction] =
      make_action_line("Main Menu", config.menu_position + config.main_menu_offset,
                       config.main_menu_scale);
  relayout_actions_for_variable_heights();
  refresh_audio_action_label();

  pause_menu_icon = arena::create<ecs::Entity>();
  auto* pause_menu_icon_transform = arena::create<transform::NoRotationTransform>();
  const float icon_width = view_size.x * 0.25f;
  const glm::vec2 icon_size = shrooms::texture_sizing::from_width_px("menu_pause", icon_width);
  const glm::vec2 resume_center =
      action_lines[kResumeAction].button_base_pos + action_lines[kResumeAction].button_base_size * 0.5f;
  const glm::vec2 restart_center =
      action_lines[kRestartAction].button_base_pos + action_lines[kRestartAction].button_base_size * 0.5f;
  const float row_step = std::max(std::abs(restart_center.y - resume_center.y),
                                  action_lines[kResumeAction].button_base_size.y);
  const float icon_padding = row_step * 1.0f;
  const float icon_bottom = action_lines[kResumeAction].button_base_pos.y - icon_padding;
  pause_menu_icon_transform->pos = glm::vec2{
      resume_center.x - icon_size.x * 0.5f,
      icon_bottom - icon_size.y,
  };
  pause_menu_icon->add(pause_menu_icon_transform);
  pause_menu_icon->add(arena::create<layers::ConstLayer>(config.text_layer + 1));
  const engine::TextureId pause_menu_icon_tex = engine::resources::register_texture("menu_pause");
  pause_menu_icon->add(arena::create<render_system::SpriteRenderable>(
      pause_menu_icon_tex, icon_size, engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f}));
  pause_menu_icon_hidden = arena::create<hidden::HiddenObject>();
  pause_menu_icon->add(pause_menu_icon_hidden);
  pause_menu_icon_hidden->hide();
  pause_menu_icon->add(arena::create<scene::SceneObject>("main"));

  pause_toggle_button = arena::create<ecs::Entity>();
  pause_toggle_transform = arena::create<transform::NoRotationTransform>();
  const glm::vec2 pause_button_size = shrooms::screen::scale_to_pixels(config.pause_button_scale);
  const glm::vec2 pause_button_center = shrooms::screen::norm_to_pixels(config.pause_button_position);
  pause_toggle_transform->pos =
      shrooms::screen::center_to_top_left(pause_button_center, pause_button_size);
  pause_toggle_base_pos = pause_toggle_transform->pos;
  pause_toggle_base_size = pause_button_size;
  pause_toggle_button->add(pause_toggle_transform);
  pause_toggle_button->add(arena::create<layers::ConstLayer>(config.pause_button_layer));
  pause_toggle_quad = arena::create<render_system::QuadRenderable>(
      pause_button_size.x, pause_button_size.y, pause_toggle_base_color);
  pause_toggle_button->add(pause_toggle_quad);
  pause_toggle_button_hidden = arena::create<hidden::HiddenObject>();
  pause_toggle_button->add(pause_toggle_button_hidden);
  pause_toggle_button_hidden->hide();
  pause_toggle_button->add(arena::create<scene::SceneObject>("main"));

  pause_toggle_icon = arena::create<ecs::Entity>();
  auto* pause_toggle_icon_transform = arena::create<transform::NoRotationTransform>();
  const glm::vec2 pause_icon_size = shrooms::screen::scale_to_pixels(config.pause_icon_scale);
  pause_toggle_icon_transform->pos =
      shrooms::screen::center_to_top_left(pause_button_center, pause_icon_size);
  pause_toggle_icon->add(pause_toggle_icon_transform);
  pause_toggle_icon->add(arena::create<layers::ConstLayer>(config.pause_button_layer + 1));
  const engine::TextureId pause_toggle_icon_tex =
      engine::resources::register_texture(kPauseToggleIconTexture);
  pause_toggle_icon_sprite = arena::create<render_system::SpriteRenderable>(
      pause_toggle_icon_tex, pause_icon_size, engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f});
  pause_toggle_icon->add(pause_toggle_icon_sprite);
  pause_toggle_icon_hidden = arena::create<hidden::HiddenObject>();
  pause_toggle_icon->add(pause_toggle_icon_hidden);
  pause_toggle_icon_hidden->hide();
  pause_toggle_icon->add(arena::create<scene::SceneObject>("main"));
}

}  // namespace pause_menu
