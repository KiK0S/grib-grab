#pragma once

#include <algorithm>
#include <array>
#include <string_view>
#include <ctime>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/color/color_object.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/text/text_object.hpp"
#include "systems/input/input_system.hpp"
#include "systems/render/render_system.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/transformation/transform_object.hpp"
#include "systems/hidden/hidden_object.hpp"

#include "countdown.hpp"
#include "level_intro.hpp"
#include "leaderboard.hpp"
#include "level_manager.hpp"
#include "score_hud.hpp"
#include "controls.hpp"
#include "player.hpp"
#include "share_bridge.hpp"
#include "tutorial.hpp"
#include "camera_shake.hpp"
#include "game_audio.hpp"
#include "vfx.hpp"
#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"
#include "shrooms_scenes.hpp"
#include "systems/text_input/text_input_system.hpp"

namespace menu {

constexpr size_t kMaxLevelLines = 10;
constexpr float kMenuTextX = -0.8f;
constexpr float kLeaderboardTextX = kMenuTextX;
constexpr float kMenuTextYOffsetNorm = 0.1f;
constexpr float kModeSwitchCenterX = 0.44f;
constexpr float kModeSwitchCenterY = 0.28f;
constexpr size_t kLeaderboardLines = static_cast<size_t>(leaderboard::kMaxEntries);
constexpr size_t kNameMaxLength = 12;
constexpr const char* kDefaultMenuBackgroundTexture = "background";
constexpr float kMenuLineIconWidthPx = 34.0f;
constexpr float kMenuLineIconGapPx = 8.0f;
constexpr int kKeyArrowUpDom = 38;
constexpr int kKeyArrowDownDom = 40;
constexpr int kKeyArrowLeftDom = 37;
constexpr int kKeyArrowRightDom = 39;
constexpr int kKeyArrowUpSdl = 1073741906;
constexpr int kKeyArrowDownSdl = 1073741905;
constexpr int kKeyArrowLeftSdl = 1073741904;
constexpr int kKeyArrowRightSdl = 1073741903;
constexpr float kVolumeAdjustStep = 0.05f;

struct LineTextColor : public color::ColoredObject {
  explicit LineTextColor(const glm::vec4& value) : color::ColoredObject(), value(value) {}
  ~LineTextColor() override { Component::component_count--; }
  glm::vec4 get_color() override { return value; }
  glm::vec4 value{1.0f, 1.0f, 1.0f, 1.0f};
};

struct TextLine {
  ecs::Entity* entity = nullptr;
  transform::NoRotationTransform* transform = nullptr;
  text::TextObject* text_object = nullptr;
  hidden::HiddenObject* text_hidden = nullptr;
  LineTextColor* text_color = nullptr;
  ecs::Entity* icon_entity = nullptr;
  transform::NoRotationTransform* icon_transform = nullptr;
  render_system::SpriteRenderable* icon_sprite = nullptr;
  hidden::HiddenObject* icon_hidden = nullptr;
  glm::vec2 icon_size{0.0f, 0.0f};
  std::string icon_texture_name{};
  float icon_gap_px = kMenuLineIconGapPx;
  ecs::Entity* button_entity = nullptr;
  transform::NoRotationTransform* button_transform = nullptr;
  render_system::QuadRenderable* button_quad = nullptr;
  hidden::HiddenObject* button_hidden = nullptr;
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
  glm::vec2 anchor_pos{0.0f, 0.0f};
  glm::vec2 size{0.0f, 0.0f};
  glm::vec2 button_size{0.0f, 0.0f};
  glm::vec2 button_base_pos{0.0f, 0.0f};
  glm::vec2 button_base_size{0.0f, 0.0f};
  glm::vec2 slider_track_pos{0.0f, 0.0f};
  glm::vec2 slider_track_size{0.0f, 0.0f};
  float slider_value = 1.0f;
  engine::UIColor base_button_color{0.0f, 0.0f, 0.0f, 0.0f};
  engine::UIColor hover_button_color{0.2f, 0.15f, 0.3f, 0.85f};
  engine::UIColor selected_button_color{0.15f, 0.15f, 0.15f, 0.95f};
  engine::UIColor dimmed_button_color{0.1f, 0.1f, 0.1f, 0.5f};
  engine::UIColor slider_track_color{0.82f, 0.84f, 0.88f, 0.98f};
  engine::UIColor slider_fill_color{0.11f, 0.66f, 0.94f, 1.0f};
  engine::UIColor slider_knob_color{1.0f, 1.0f, 1.0f, 1.0f};
  engine::UIColor slider_dimmed_color{0.56f, 0.56f, 0.58f, 0.92f};
  glm::vec4 base_text_color{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 selected_text_color{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 dimmed_text_color{0.74f, 0.74f, 0.74f, 0.92f};
  glm::vec4 locked_text_color{0.32f, 0.32f, 0.32f, 0.78f};
  float hover_scale = 1.04f;
  float selected_scale = 1.0f;
  float font_px = 18.0f;
  bool center_on_anchor = false;
  bool icon_before_text = false;
};

inline TextLine status_line{};
inline TextLine instruction_line{};
inline TextLine mode_line{};
inline TextLine mode_recipe_line{};
inline TextLine settings_line{};
inline TextLine audio_line{};
inline TextLine tutorial_line{};
inline TextLine credits_line{};
inline std::array<TextLine, kMaxLevelLines> level_lines{};
inline size_t active_level_lines = 0;

inline ecs::Entity* menu_background = nullptr;
inline render_system::SpriteRenderable* menu_background_sprite = nullptr;
inline transform::NoRotationTransform* menu_background_transform = nullptr;

enum class MenuMode {
  Main,
  Settings,
  Objective,
  GameOver,
};

inline MenuMode menu_mode = MenuMode::Main;

inline TextLine objective_title{};
inline TextLine objective_level{};
inline TextLine objective_hint{};
inline std::array<TextLine, kMaxLevelLines> objective_recipe_lines{};
inline size_t active_objective_lines = 0;

inline TextLine settings_title{};
inline TextLine settings_hint{};
inline std::array<TextLine, controls::kActionCount> settings_control_lines{};
inline TextLine settings_reset{};
inline TextLine settings_back{};
inline std::optional<controls::Action> pending_control_remap{};
inline std::string settings_hint_message{};

inline TextLine gameover_title{};
inline TextLine gameover_level{};
inline TextLine gameover_collected{};
inline TextLine gameover_sorted{};
inline TextLine gameover_score{};
inline TextLine gameover_hint{};
inline TextLine gameover_name_prompt{};
inline TextLine gameover_name_value{};
inline TextLine gameover_restart{};
inline TextLine gameover_main_menu{};
inline TextLine gameover_share{};
inline TextLine leaderboard_title{};
inline std::array<TextLine, kLeaderboardLines> leaderboard_lines{};

inline size_t pending_level_index = 0;
inline bool has_pending_level = false;
inline bool pending_infinite = false;
inline bool pending_tutorial = false;
inline bool awaiting_name_entry = false;
inline bool show_leaderboard = false;
inline int pending_leaderboard_score = 0;
inline int block_input_frames = 0;

inline void suppress_input_for_frames(int frames = 2) {
  block_input_frames = std::max(block_input_frames, frames);
}

inline void enter_objective_mode(size_t level_index);
inline void enter_infinite_objective_mode();
inline void enter_tutorial_objective_mode();
inline void enter_game_over_mode();
inline void enter_main_menu_mode();
inline void enter_settings_mode();
inline void begin_started_level_intro(const std::string& title, bool recipe_first = false);
inline void start_level_from_menu(size_t level_index);
inline void start_infinite_from_menu();

inline std::string infinite_background_texture() {
  if (!levels::parsed_levels.empty()) {
    return levels::parsed_levels.back().id;
  }
  return kDefaultMenuBackgroundTexture;
}

inline std::string objective_background_texture() {
  if (pending_tutorial) {
    return kDefaultMenuBackgroundTexture;
  }
  if (pending_infinite) {
    return infinite_background_texture();
  }
  if (pending_level_index < levels::parsed_levels.size()) {
    return levels::parsed_levels[pending_level_index].id;
  }
  return kDefaultMenuBackgroundTexture;
}

inline std::string gameover_background_texture() {
  if (!levels::last_result_valid || levels::last_result.tutorial_mode) {
    return kDefaultMenuBackgroundTexture;
  }
  if (levels::last_result.infinite_mode) {
    return infinite_background_texture();
  }
  if (levels::last_result.level_index < levels::parsed_levels.size()) {
    return levels::parsed_levels[levels::last_result.level_index].id;
  }
  if (!levels::last_result.level_id.empty()) {
    return levels::last_result.level_id;
  }
  return kDefaultMenuBackgroundTexture;
}

inline std::string current_date_ymd() {
  const std::time_t now = std::time(nullptr);
  std::tm local_tm{};
#if defined(_WIN32)
  localtime_s(&local_tm, &now);
#else
  localtime_r(&now, &local_tm);
#endif
  std::ostringstream out;
  out << std::put_time(&local_tm, "%Y-%m-%d");
  return out.str();
}

inline std::string build_share_text(int score) {
  constexpr const char* kMushroomEmoji = "\xF0\x9F\x8D\x84";
  return std::string(kMushroomEmoji) + " grib grab Infinite Score: " + std::to_string(score) + " " +
         kMushroomEmoji + "\nPlay: https://kik0s.github.io/shrooms\nDate: " + current_date_ymd();
}

inline std::string infinity_objective_label() {
  const int round = std::max(1, levels::infinite_round_index + 1);
  const int score = std::max(0, levels::score());
  return "Daily Infinity Mode: Round " + std::to_string(round) + " (score " +
         std::to_string(score) + ")";
}

inline void apply_menu_background(const std::string& texture_name, bool level_layout) {
  if (!menu_background_sprite || !menu_background_transform) return;

  std::string resolved = texture_name.empty() ? kDefaultMenuBackgroundTexture : texture_name;
  engine::TextureId tex_id = engine::resources::register_texture(resolved);
  if (tex_id == engine::kInvalidTextureId && resolved != kDefaultMenuBackgroundTexture) {
    resolved = kDefaultMenuBackgroundTexture;
    tex_id = engine::resources::register_texture(resolved);
  }
  if (tex_id == engine::kInvalidTextureId) return;

  const float view_width = static_cast<float>(shrooms::screen::view_width);
  const float view_height = static_cast<float>(shrooms::screen::view_height);

  glm::vec2 size{};
  glm::vec2 pos{};
  if (level_layout) {
    size = shrooms::texture_sizing::from_width_px(resolved, view_width);
    pos = glm::vec2{0.0f, view_height - size.y};
  } else {
    const float side = std::min(view_width, view_height);
    size = glm::vec2{side, side};
    pos = glm::vec2{(view_width - side) * 0.5f, (view_height - side) * 0.5f};
  }

  menu_background_sprite->texture_id = tex_id;
  menu_background_sprite->geometry = engine::geometry::make_quad(size.x, size.y);
  menu_background_sprite->uploaded = false;
  menu_background_sprite->size = size;
  menu_background_transform->pos = pos;
}

inline void refresh_menu_background() {
  apply_menu_background(kDefaultMenuBackgroundTexture, false);
}

inline int line_layer(const TextLine& line) {
  if (line.entity) {
    if (auto* layer = line.entity->get<layers::LayeredObject>()) {
      return layer->get_layer();
    }
  }
  return 6;
}

inline void ensure_line_icon(TextLine& line) {
  if (line.icon_entity || !line.transform) return;
  line.icon_entity = arena::create<ecs::Entity>();
  line.icon_transform = arena::create<transform::NoRotationTransform>();
  line.icon_transform->pos = line.transform->pos;
  line.icon_entity->add(line.icon_transform);
  line.icon_entity->add(arena::create<layers::ConstLayer>(line_layer(line)));
  line.icon_hidden = arena::create<hidden::HiddenObject>();
  line.icon_entity->add(line.icon_hidden);
  line.icon_entity->add(arena::create<scene::SceneObject>("menu"));
}

inline void set_line_icon_texture(TextLine& line, const std::string& texture_name) {
  if (texture_name.empty()) {
    line.icon_texture_name.clear();
    line.icon_size = glm::vec2{0.0f, 0.0f};
    if (line.icon_hidden) {
      line.icon_hidden->hide();
    }
    return;
  }

  ensure_line_icon(line);
  if (!line.icon_entity || !line.icon_transform) return;

  const engine::TextureId tex_id = engine::resources::register_texture(texture_name);
  if (tex_id == engine::kInvalidTextureId) {
    line.icon_texture_name.clear();
    line.icon_size = glm::vec2{0.0f, 0.0f};
    if (line.icon_hidden) {
      line.icon_hidden->hide();
    }
    return;
  }

  const glm::vec2 icon_size =
      shrooms::texture_sizing::from_width_px(texture_name, kMenuLineIconWidthPx);
  if (!line.icon_sprite) {
    line.icon_sprite = arena::create<render_system::SpriteRenderable>(tex_id, icon_size);
    line.icon_entity->add(line.icon_sprite);
  } else {
    line.icon_sprite->texture_id = tex_id;
    line.icon_sprite->size = icon_size;
    line.icon_sprite->geometry = engine::geometry::make_quad(icon_size.x, icon_size.y);
    line.icon_sprite->uploaded = false;
  }

  line.icon_texture_name = texture_name;
  line.icon_size = icon_size;
  if (line.icon_hidden) {
    line.icon_hidden->show();
  }
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

inline void update_line_slider_geometry(TextLine& line) {
  if (!line.slider_track_quad || !line.slider_fill_quad || !line.slider_knob_quad ||
      !line.slider_track_transform || !line.slider_fill_transform || !line.slider_knob_transform ||
      !line.button_transform || !line.button_quad) {
    return;
  }

  const glm::vec2 button_pos = line.button_transform->pos;
  const glm::vec2 button_size{line.button_quad->width, line.button_quad->height};
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
  const float t = clamp_unit(line.slider_value);

  line.slider_track_pos = glm::vec2{track_x, track_y};
  line.slider_track_size = glm::vec2{track_w, track_h};

  line.slider_track_transform->pos = line.slider_track_pos;
  set_quad_size(line.slider_track_quad, track_w, track_h);

  line.slider_fill_transform->pos = line.slider_track_pos;
  set_quad_size(line.slider_fill_quad, track_w * t, track_h);

  const float knob_center_x = track_x + track_w * t;
  const float knob_center_y = track_y + track_h * 0.5f;
  line.slider_knob_transform->pos =
      glm::vec2{knob_center_x - knob_size * 0.5f, knob_center_y - knob_size * 0.5f};
  set_quad_size(line.slider_knob_quad, knob_size, knob_size);
}

inline void ensure_line_slider(TextLine& line) {
  if (line.slider_track_entity || !line.button_entity) return;

  const int text_layer = line_layer(line);
  const int slider_layer = text_layer - 1;
  line.slider_track_entity = arena::create<ecs::Entity>();
  line.slider_track_transform = arena::create<transform::NoRotationTransform>();
  line.slider_track_entity->add(line.slider_track_transform);
  line.slider_track_entity->add(arena::create<layers::ConstLayer>(slider_layer));
  line.slider_track_quad = arena::create<render_system::QuadRenderable>(0.0f, 0.0f, line.slider_track_color);
  line.slider_track_entity->add(line.slider_track_quad);
  line.slider_track_hidden = arena::create<hidden::HiddenObject>();
  line.slider_track_entity->add(line.slider_track_hidden);
  line.slider_track_hidden->hide();
  line.slider_track_entity->add(arena::create<scene::SceneObject>("menu"));

  line.slider_fill_entity = arena::create<ecs::Entity>();
  line.slider_fill_transform = arena::create<transform::NoRotationTransform>();
  line.slider_fill_entity->add(line.slider_fill_transform);
  line.slider_fill_entity->add(arena::create<layers::ConstLayer>(slider_layer));
  line.slider_fill_quad = arena::create<render_system::QuadRenderable>(0.0f, 0.0f, line.slider_fill_color);
  line.slider_fill_entity->add(line.slider_fill_quad);
  line.slider_fill_hidden = arena::create<hidden::HiddenObject>();
  line.slider_fill_entity->add(line.slider_fill_hidden);
  line.slider_fill_hidden->hide();
  line.slider_fill_entity->add(arena::create<scene::SceneObject>("menu"));

  line.slider_knob_entity = arena::create<ecs::Entity>();
  line.slider_knob_transform = arena::create<transform::NoRotationTransform>();
  line.slider_knob_entity->add(line.slider_knob_transform);
  line.slider_knob_entity->add(arena::create<layers::ConstLayer>(slider_layer));
  line.slider_knob_quad = arena::create<render_system::QuadRenderable>(0.0f, 0.0f, line.slider_knob_color);
  line.slider_knob_entity->add(line.slider_knob_quad);
  line.slider_knob_hidden = arena::create<hidden::HiddenObject>();
  line.slider_knob_entity->add(line.slider_knob_hidden);
  line.slider_knob_hidden->hide();
  line.slider_knob_entity->add(arena::create<scene::SceneObject>("menu"));

  update_line_slider_geometry(line);
}

inline void set_line_slider_value(TextLine& line, float value) {
  line.slider_value = clamp_unit(value);
  update_line_slider_geometry(line);
}

inline void set_line_slider_visibility(TextLine& line, bool visible) {
  if (line.slider_track_hidden) line.slider_track_hidden->set_visible(visible);
  if (line.slider_fill_hidden) line.slider_fill_hidden->set_visible(visible);
  if (line.slider_knob_hidden) line.slider_knob_hidden->set_visible(visible);
}

inline void apply_line_slider_visual(TextLine& line, bool selected, bool hovered, bool dimmed) {
  if (!line.slider_track_quad || !line.slider_fill_quad || !line.slider_knob_quad) return;
  if (dimmed) {
    line.slider_track_quad->color = line.slider_dimmed_color;
    line.slider_fill_quad->color = line.slider_fill_color;
    line.slider_knob_quad->color = engine::UIColor{0.9f, 0.9f, 0.9f, 1.0f};
    return;
  }
  line.slider_track_quad->color = line.slider_track_color;
  line.slider_fill_quad->color = line.slider_fill_color;
  line.slider_knob_quad->color =
      (selected || hovered) ? engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f} : line.slider_knob_color;
}

inline void update_text(TextLine& line, const std::string& value) {
  if (!line.text_object) return;
  line.text_object->text = value;
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, line.font_px);
  const glm::vec2 text_size{layout.width, layout.height};
  const bool has_icon = !line.icon_texture_name.empty() && line.icon_size.x > 0.0f &&
                        line.icon_size.y > 0.0f && line.icon_transform;
  const float icon_w = has_icon ? line.icon_size.x : 0.0f;
  const float icon_h = has_icon ? line.icon_size.y : 0.0f;
  const float text_x = (has_icon && line.icon_before_text) ? (icon_w + line.icon_gap_px) : 0.0f;
  const float icon_x =
      has_icon ? (line.icon_before_text ? 0.0f : (text_size.x + line.icon_gap_px)) : 0.0f;
  const float icon_y = has_icon ? ((text_size.y - icon_h) * 0.5f) : 0.0f;
  const float content_top = has_icon ? std::min(0.0f, icon_y) : 0.0f;
  const float content_right =
      has_icon ? std::max(text_x + text_size.x, icon_x + icon_w) : text_size.x;
  const float content_bottom = has_icon ? std::max(text_size.y, icon_y + icon_h) : text_size.y;
  line.size = glm::vec2{std::max(0.0f, content_right), std::max(0.0f, content_bottom - content_top)};
  const glm::vec2 content_origin =
      line.center_on_anchor ? (line.anchor_pos - line.size * 0.5f)
                            : (line.anchor_pos + glm::vec2{0.0f, content_top});

  if (line.transform) {
    line.transform->pos =
        content_origin + glm::vec2{text_x, -content_top};
  }

  if (has_icon && line.icon_transform) {
    line.icon_transform->pos =
        content_origin + glm::vec2{icon_x, icon_y - content_top};
  }

  if (line.button_transform && line.button_quad) {
    const float pad_x = 14.0f;
    const float pad_y = 8.0f;
    const bool slider_mode = line.slider_track_entity != nullptr;
    const float slider_min_w = slider_mode ? 220.0f : 0.0f;
    const float slider_extra_h = slider_mode ? 26.0f : 0.0f;
    line.button_size = glm::vec2{
        std::max(line.size.x + pad_x * 2.0f, slider_min_w),
        line.size.y + pad_y * 2.0f + slider_extra_h,
    };
    line.button_base_size = line.button_size;
    line.button_base_pos = content_origin - glm::vec2{pad_x, pad_y};
    line.button_transform->pos = line.button_base_pos;
    set_quad_size(line.button_quad, line.button_base_size.x, line.button_base_size.y);
    line.button_quad->color = line.base_button_color;

    if (slider_mode && line.transform && !has_icon) {
      const float top_pad = std::max(6.0f, line.button_base_size.y * 0.10f);
      const float bottom_pad = std::max(6.0f, line.button_base_size.y * 0.10f);
      const float inner_h = std::max(1.0f, line.button_base_size.y - top_pad - bottom_pad);
      const float text_band_h = inner_h * 0.60f;
      line.transform->pos = glm::vec2{
          line.button_base_pos.x + (line.button_base_size.x - text_size.x) * 0.5f,
          line.button_base_pos.y + top_pad + (text_band_h - text_size.y) * 0.5f,
      };
    }
  }
  update_line_slider_geometry(line);
}

inline void set_line_visibility(TextLine& line, bool text_visible, bool button_visible) {
  if (line.text_hidden) {
    line.text_hidden->set_visible(text_visible);
  }
  if (line.icon_hidden) {
    line.icon_hidden->set_visible(text_visible && !line.icon_texture_name.empty());
  }
  if (line.button_hidden) {
    line.button_hidden->set_visible(button_visible);
  }
  set_line_slider_visibility(line, button_visible && line.slider_track_entity);
}

inline void apply_button_visual(TextLine& line, float scale, const engine::UIColor& color) {
  if (!line.button_transform || !line.button_quad) return;
  const glm::vec2 size = line.button_base_size * scale;
  const glm::vec2 center = line.button_base_pos + line.button_base_size * 0.5f;
  line.button_transform->pos = center - size * 0.5f;
  set_quad_size(line.button_quad, size.x, size.y);
  line.button_quad->color = color;
  update_line_slider_geometry(line);
}

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

inline void set_line_visual_state(TextLine& line,
                                  bool selected,
                                  bool hovered,
                                  bool dimmed,
                                  bool locked = false) {
  const glm::vec4 text_color =
      selected ? line.selected_text_color
               : (locked ? line.locked_text_color
                         : (dimmed ? line.dimmed_text_color : line.base_text_color));
  if (line.text_color) {
    line.text_color->value = text_color;
  }
  if (line.icon_sprite) {
    const glm::vec4 icon_color = locked ? line.base_text_color : text_color;
    line.icon_sprite->tint =
        engine::UIColor{icon_color.x, icon_color.y, icon_color.z, icon_color.w};
  }
  if (!line.button_quad) return;
  if (selected) {
    apply_line_slider_visual(line, selected, hovered, dimmed);
    apply_button_visual(line, line.selected_scale, line.selected_button_color);
    return;
  }
  if (hovered) {
    apply_line_slider_visual(line, selected, hovered, dimmed);
    apply_button_visual(line, line.hover_scale, line.hover_button_color);
    return;
  }
  if (dimmed) {
    apply_line_slider_visual(line, selected, hovered, dimmed);
    apply_button_visual(line, 1.0f, line.dimmed_button_color);
    return;
  }
  apply_line_slider_visual(line, selected, hovered, dimmed);
  apply_button_visual(line, 1.0f, line.base_button_color);
}

inline void set_line_hovered(TextLine& line, bool hovered) {
  set_line_visual_state(line, false, hovered, false);
}

inline void set_entity_visible(ecs::Entity* entity, bool visible) {
  if (!entity) return;
  auto* hidden = entity->get<hidden::HiddenObject>();
  if (!hidden) return;
  hidden->set_visible(visible);
}

inline bool show_infinite_entry() {
  return levels::infinite_available();
}

inline size_t available_levels() {
  size_t count = levels::parsed_levels.size();
  if (show_infinite_entry()) {
    count += 1;
  }
  return std::min(kMaxLevelLines, count);
}

inline bool is_infinite_entry(size_t index) {
  return show_infinite_entry() && index == levels::infinite_menu_index();
}

inline bool is_selectable_level(size_t index) {
  if (index >= active_level_lines) return false;
  return is_infinite_entry(index) || levels::is_unlocked(index);
}

inline std::string display_level_name(const std::string& level_id) {
  std::string_view view{level_id};
  constexpr std::string_view kLevelPrefix = "level_";
  if (!view.starts_with(kLevelPrefix)) {
    return level_id;
  }

  size_t cursor = kLevelPrefix.size();
  if (cursor >= view.size() || view[cursor] < '0' || view[cursor] > '9') {
    return level_id;
  }
  while (cursor < view.size() && view[cursor] >= '0' && view[cursor] <= '9') {
    ++cursor;
  }
  if (cursor >= view.size() || view[cursor] != '_') {
    return level_id;
  }
  ++cursor;
  if (cursor >= view.size()) {
    return level_id;
  }
  return std::string(view.substr(cursor));
}

inline std::string format_level_line(size_t index) {
  if (index >= active_level_lines) return "";
  return std::to_string(index + 1) + ".";
}

inline std::string base_level_icon_texture(size_t index) {
  if (is_infinite_entry(index)) {
    return "emoji_infinity";
  }
  switch (index) {
    case 0:
      return "emoji_hedgehog";
    case 1:
      return "emoji_tree";
    case 2:
      return "emoji_house";
    case 3:
      return "emoji_frog";
    case 4:
      return "emoji_fly";
    case 5:
      return "emoji_crown";
    case 6:
      return "emoji_strawberry";
    default:
      return "";
  }
}

inline std::string level_icon_texture(size_t index) {
  if (index >= levels::parsed_levels.size()) {
    return base_level_icon_texture(index);
  }
  if (!levels::is_unlocked(index)) {
    return "emoji_lock";
  }
  return base_level_icon_texture(index);
}

inline void refresh_status_line() {
  update_text(status_line, "");
}

inline void refresh_instruction_line() {
  update_text(instruction_line, "");
}

inline glm::vec2 mode_switch_segment_size() {
  const float width =
      std::clamp(static_cast<float>(shrooms::screen::view_width) * 0.12f, 86.0f, 122.0f);
  const float height =
      std::clamp(static_cast<float>(shrooms::screen::view_height) * 0.052f, 44.0f, 54.0f);
  return glm::vec2{width, height};
}

inline void set_line_fixed_button(TextLine& line,
                                  const glm::vec2& top_left,
                                  const glm::vec2& size) {
  line.anchor_pos = top_left;
  line.size = size;
  line.button_size = size;
  line.button_base_pos = top_left;
  line.button_base_size = size;
  if (line.button_transform) {
    line.button_transform->pos = top_left;
  }
  if (line.button_quad) {
    set_quad_size(line.button_quad, size.x, size.y);
  }
  if (line.transform && line.text_object) {
    const auto layout =
        engine::text::layout_text(line.text_object->text, 0.0f, 0.0f, line.font_px);
    line.transform->pos = top_left + glm::vec2{
                                          (size.x - layout.width) * 0.5f,
                                          (size.y - layout.height) * 0.5f,
                                      };
  }
}

inline void layout_mode_switch() {
  const glm::vec2 segment_size = mode_switch_segment_size();
  const float gap_px = std::clamp(segment_size.x * 0.035f, 4.0f, 6.0f);
  const float total_width = segment_size.x * 3.0f + gap_px * 2.0f;
  const glm::vec2 center = shrooms::screen::norm_to_pixels(
      glm::vec2{kModeSwitchCenterX, kModeSwitchCenterY + kMenuTextYOffsetNorm});
  const glm::vec2 top_left = center - glm::vec2{total_width, segment_size.y} * 0.5f;
  set_line_fixed_button(mode_line, top_left, segment_size);
  set_line_fixed_button(mode_recipe_line,
                        top_left + glm::vec2{segment_size.x + gap_px, 0.0f},
                        segment_size);
  set_line_fixed_button(tutorial_line,
                        top_left + glm::vec2{(segment_size.x + gap_px) * 2.0f, 0.0f},
                        segment_size);
}

inline engine::UIColor mode_switch_active_color(levels::GameMode mode) {
  switch (mode) {
    case levels::GameMode::Collector:
      return engine::UIColor{0.08f, 0.48f, 0.30f, 0.96f};
    case levels::GameMode::Recipe:
    default:
      return engine::UIColor{0.72f, 0.34f, 0.14f, 0.96f};
  }
}

inline engine::UIColor tutorial_switch_color() {
  return engine::UIColor{0.22f, 0.35f, 0.62f, 0.96f};
}

inline void style_mode_switch_segment(TextLine& line,
                                      const engine::UIColor& accent_color,
                                      bool active) {
  const engine::UIColor inactive_color{0.08f, 0.09f, 0.10f, 0.88f};
  const engine::UIColor inactive_hover{0.18f, 0.20f, 0.23f, 0.96f};

  line.base_button_color = active ? accent_color : inactive_color;
  line.hover_button_color = active ? accent_color : inactive_hover;
  line.selected_button_color = active ? accent_color : inactive_hover;
  line.dimmed_button_color =
      active ? engine::UIColor{accent_color.r, accent_color.g, accent_color.b, 0.74f}
             : engine::UIColor{0.06f, 0.06f, 0.07f, 0.58f};
  line.base_text_color = active ? glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}
                                : glm::vec4{0.76f, 0.78f, 0.80f, 0.95f};
  line.selected_text_color = active ? glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}
                                    : glm::vec4{0.84f, 0.86f, 0.88f, 1.0f};
  line.dimmed_text_color = active ? glm::vec4{0.92f, 0.94f, 0.94f, 0.82f}
                                  : glm::vec4{0.50f, 0.52f, 0.54f, 0.78f};
  line.hover_scale = 1.02f;
  line.selected_scale = 1.0f;
}

inline void set_mode_switch_visual_state(bool collector_selected,
                                         bool recipe_selected,
                                         bool tutorial_selected,
                                         bool collector_hovered,
                                         bool recipe_hovered,
                                         bool tutorial_hovered,
                                         bool dimmed) {
  const bool collector_active = levels::game_mode() == levels::GameMode::Collector;
  style_mode_switch_segment(mode_line, mode_switch_active_color(levels::GameMode::Collector),
                            collector_active);
  style_mode_switch_segment(mode_recipe_line, mode_switch_active_color(levels::GameMode::Recipe),
                            !collector_active);
  style_mode_switch_segment(tutorial_line, tutorial_switch_color(), false);
  set_line_visual_state(mode_line, collector_selected, collector_hovered, dimmed);
  set_line_visual_state(mode_recipe_line, recipe_selected, recipe_hovered, dimmed);
  set_line_visual_state(tutorial_line, tutorial_selected, tutorial_hovered, dimmed);
}

inline void refresh_mode_line() {
  update_text(mode_line, "Collector");
  update_text(mode_recipe_line, "Recipe");
  layout_mode_switch();
  set_mode_switch_visual_state(false, false, false, false, false, false, false);
}

inline void refresh_settings_line() {
  update_text(settings_line, "Settings");
}

inline void refresh_audio_line() {
  update_text(audio_line, shrooms::audio::volume_label());
  set_line_slider_value(audio_line, shrooms::audio::volume_slider_value());
}

inline void refresh_tutorial_line() {
  update_text(tutorial_line, "Tutorial");
  layout_mode_switch();
  set_mode_switch_visual_state(false, false, false, false, false, false, false);
}

inline std::string settings_control_line_text(controls::Action action) {
  if (pending_control_remap && *pending_control_remap == action) {
    return std::string(controls::action_label(action)) + ": Press key";
  }
  return std::string(controls::action_label(action)) + ": " + controls::bound_key_label(action);
}

inline void refresh_settings_lines() {
  update_text(settings_title, "Settings");
  const std::string default_hint =
      controls::is_mobile_layout()
          ? "Controls are read-only on mobile"
          : "Click a control, then press a letter, number, or arrow";
  update_text(settings_hint, settings_hint_message.empty() ? default_hint : settings_hint_message);
  refresh_audio_line();
  for (size_t i = 0; i < settings_control_lines.size(); ++i) {
    const auto action = static_cast<controls::Action>(i);
    update_text(settings_control_lines[i], settings_control_line_text(action));
  }
  update_text(settings_reset, "Reset Controls");
  update_text(settings_back, "Back");
}

inline void refresh_level_lines() {
  active_level_lines = available_levels();
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    if (i < active_level_lines) {
      set_line_icon_texture(level_lines[i], level_icon_texture(i));
      update_text(level_lines[i], format_level_line(i));
    } else {
      set_line_icon_texture(level_lines[i], "");
      update_text(level_lines[i], "");
    }
  }
}

inline void relayout_main_rows_for_variable_heights() {
  constexpr float gap_px = 12.0f;
  std::array<TextLine*, kMaxLevelLines + 1> ordered{};
  size_t count = 0;
  ordered[count++] = &settings_line;
  for (size_t i = 0; i < active_level_lines && i < kMaxLevelLines; ++i) {
    ordered[count++] = &level_lines[i];
  }

  if (count == 0 || !ordered[0] || !ordered[0]->button_transform) return;
  float next_top = ordered[0]->button_base_pos.y;
  for (size_t i = 0; i < count; ++i) {
    auto* line = ordered[i];
    if (!line || !line->button_transform || !line->text_object || !line->transform) continue;
    if (i > 0 && line->button_base_pos.y < next_top) {
      const float delta_y = next_top - line->button_base_pos.y;
      line->anchor_pos.y += delta_y;
      line->transform->pos.y += delta_y;
      update_text(*line, line->text_object->text);
    }
    next_top = line->button_base_pos.y + line->button_base_size.y + gap_px;
  }
}

inline void relayout_settings_rows_for_variable_heights() {
  constexpr float gap_px = 12.0f;
  std::array<TextLine*, controls::kActionCount + 3> ordered{};
  size_t count = 0;
  ordered[count++] = &audio_line;
  for (auto& line : settings_control_lines) {
    ordered[count++] = &line;
  }
  ordered[count++] = &settings_reset;
  ordered[count++] = &settings_back;

  if (count == 0 || !ordered[0] || !ordered[0]->button_transform) return;
  float next_top = ordered[0]->button_base_pos.y;
  for (size_t i = 0; i < count; ++i) {
    auto* line = ordered[i];
    if (!line || !line->button_transform || !line->text_object || !line->transform) continue;
    if (i > 0 && line->button_base_pos.y < next_top) {
      const float delta_y = next_top - line->button_base_pos.y;
      line->anchor_pos.y += delta_y;
      line->transform->pos.y += delta_y;
      update_text(*line, line->text_object->text);
    }
    next_top = line->button_base_pos.y + line->button_base_size.y + gap_px;
  }
}

inline bool point_hits_line(const TextLine& line, const glm::vec2& point) {
  if (!line.transform && !line.button_transform) return false;
  const glm::vec2 pos = line.button_transform ? line.button_base_pos : line.transform->pos;
  const glm::vec2 size = line.button_transform ? line.button_base_size : line.size;
  if (point.x < pos.x || point.x > pos.x + size.x) return false;
  if (point.y < pos.y || point.y > pos.y + size.y) return false;
  return true;
}

inline bool point_hits_slider(const TextLine& line, const glm::vec2& point) {
  if (!line.slider_track_quad) return false;
  const float extra_y = 10.0f;
  const glm::vec2 min_bound{line.slider_track_pos.x, line.slider_track_pos.y - extra_y};
  const glm::vec2 max_bound{line.slider_track_pos.x + line.slider_track_size.x,
                            line.slider_track_pos.y + line.slider_track_size.y + extra_y};
  if (point.x < min_bound.x || point.x > max_bound.x) return false;
  if (point.y < min_bound.y || point.y > max_bound.y) return false;
  return true;
}

inline std::optional<size_t> level_index_at_point(const glm::vec2& point) {
  size_t count = active_level_lines;
  for (size_t i = 0; i < count; ++i) {
    const auto& line = level_lines[i];
    if (point_hits_line(line, point)) return i;
  }
  return std::nullopt;
}

inline void handle_level_selection(size_t index) {
  if (is_infinite_entry(index)) {
    start_infinite_from_menu();
    return;
  }
  if (index >= levels::parsed_levels.size()) {
    return;
  }
  if (!levels::is_unlocked(index)) {
    return;
  }
  start_level_from_menu(index);
}

inline void refresh_objective_lines_from_level(const levels::LevelDefinition& level,
                                               const std::string& label) {
  update_text(objective_title, "Objective");
  update_text(objective_level, label);
  active_objective_lines = std::min(kMaxLevelLines, level.recipe_order.size());
  if (levels::game_mode() == levels::GameMode::Collector) {
    active_objective_lines = 2;
  }
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    set_line_icon_texture(objective_recipe_lines[i], "");
    if (levels::game_mode() == levels::GameMode::Collector && i < active_objective_lines) {
      update_text(objective_recipe_lines[i],
                  i == 0 ? "Catch every mushroom" : "3 lives. No shooting.");
    } else if (i < active_objective_lines) {
      const auto& item = level.recipe_order[i];
      set_line_icon_texture(objective_recipe_lines[i], levels::mushroom_icon_texture(item.first));
      update_text(objective_recipe_lines[i], "x " + std::to_string(item.second));
    } else {
      update_text(objective_recipe_lines[i], "");
    }
  }
  update_text(objective_hint, "Press Enter or tap to start");
}

inline void refresh_objective_lines(size_t level_index) {
  if (level_index >= levels::parsed_levels.size()) {
    active_objective_lines = 0;
    for (size_t i = 0; i < kMaxLevelLines; ++i) {
      update_text(objective_recipe_lines[i], "");
    }
    return;
  }
  const auto& level = levels::parsed_levels[level_index];
  refresh_objective_lines_from_level(
      level, "Level " + std::to_string(level_index + 1) + ": " + display_level_name(level.id));
}

inline void refresh_leaderboard_lines() {
  if (!show_leaderboard) {
    update_text(leaderboard_title, "");
    for (auto& line : leaderboard_lines) {
      update_text(line, "");
    }
    return;
  }
  update_text(leaderboard_title, "Daily Infinite Leaderboard\n" + levels::mode_label() +
                                     ", " + leaderboard::current_date());
  const auto& entries = leaderboard::list();
  for (size_t i = 0; i < leaderboard_lines.size(); ++i) {
    if (i < entries.size()) {
      const auto& entry = entries[i];
      const std::string line = std::to_string(i + 1) + ". " + entry.name + " - " +
                               std::to_string(entry.score);
      update_text(leaderboard_lines[i], line);
    } else {
      update_text(leaderboard_lines[i], "");
    }
  }
}

inline void refresh_name_entry_lines() {
  if (!show_leaderboard || !awaiting_name_entry) {
    update_text(gameover_name_prompt, "");
    update_text(gameover_name_value, "");
    return;
  }
  update_text(gameover_name_prompt, "New high score! Enter your name:");
  std::string display = text_input::value();
  if (display.empty()) {
    display = "_";
  } else {
    display += "_";
  }
  update_text(gameover_name_value, display);
}

inline void refresh_gameover_lines() {
  if (!levels::last_result_valid) return;
  const auto& result = levels::last_result;
  show_leaderboard = result.infinite_mode;
  update_text(gameover_title, "");
  update_text(gameover_level, "");
  update_text(gameover_collected, "");
  update_text(gameover_sorted, "");
  update_text(gameover_score,
              result.infinite_mode
                  ? "Your score: " + std::to_string(std::max(0, result.global_score))
                  : "");
  update_text(gameover_hint, "");
  update_text(gameover_restart, "Restart");
  update_text(gameover_main_menu, "Main Menu");
  update_text(gameover_share, "Share");
  refresh_name_entry_lines();
  refresh_leaderboard_lines();
}

inline void set_menu_mode(MenuMode mode) {
  menu_mode = mode;
  const bool show_main = (mode == MenuMode::Main);
  const bool show_settings = (mode == MenuMode::Settings);
  const bool show_objective = (mode == MenuMode::Objective);
  const bool show_game_over = (mode == MenuMode::GameOver);

  set_line_visibility(status_line, false, false);
  set_line_visibility(instruction_line, false, false);
  set_line_visibility(mode_line, show_main, show_main);
  set_line_visibility(mode_recipe_line, show_main, show_main);
  set_line_visibility(settings_line, show_main, show_main);
  set_line_visibility(audio_line, show_settings, show_settings);
  set_line_visibility(tutorial_line, show_main, show_main);
  set_line_visibility(credits_line, show_main, false);
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    const bool show_level_row = show_main && i < active_level_lines;
    set_line_visibility(level_lines[i], show_level_row, show_level_row);
  }
  set_line_visibility(objective_title, show_objective, false);
  set_line_visibility(objective_level, show_objective, false);
  set_line_visibility(objective_hint, show_objective, false);
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    set_line_visibility(objective_recipe_lines[i], show_objective && i < active_objective_lines,
                        false);
  }
  set_line_visibility(settings_title, show_settings, false);
  set_line_visibility(settings_hint, show_settings, false);
  for (auto& line : settings_control_lines) {
    set_line_visibility(line, show_settings, show_settings);
  }
  set_line_visibility(settings_reset, show_settings, show_settings);
  set_line_visibility(settings_back, show_settings, show_settings);

  set_line_visibility(gameover_title, false, false);
  set_line_visibility(gameover_level, false, false);
  set_line_visibility(gameover_collected, false, false);
  set_line_visibility(gameover_sorted, false, false);
  set_line_visibility(gameover_score, show_game_over && levels::last_result_valid &&
                                          levels::last_result.infinite_mode,
                      false);
  const bool show_restart = show_game_over && !awaiting_name_entry;
  const bool show_share = show_restart && levels::last_result_valid && levels::last_result.infinite_mode;
  const bool show_hint =
      show_game_over && gameover_hint.text_object && !gameover_hint.text_object->text.empty();
  set_line_visibility(gameover_hint, show_hint, false);
  set_line_visibility(gameover_name_prompt, show_game_over && awaiting_name_entry, false);
  set_line_visibility(gameover_name_value, show_game_over && awaiting_name_entry, false);
  set_line_visibility(gameover_restart, show_restart, show_restart);
  set_line_visibility(gameover_main_menu, show_restart, show_restart);
  set_line_visibility(gameover_share, show_share, show_share);
  const bool show_board = show_game_over && show_leaderboard;
  set_line_visibility(leaderboard_title, show_board, false);
  for (auto& line : leaderboard_lines) {
    set_line_visibility(line, show_board, false);
  }
  refresh_menu_background();
}

inline void start_pending_level() {
  if (!has_pending_level) return;
  if (!pending_infinite && !pending_tutorial &&
      pending_level_index >= levels::parsed_levels.size()) {
    has_pending_level = false;
    pending_infinite = false;
    pending_tutorial = false;
    enter_main_menu_mode();
    return;
  }
  score_hud::reset_for_run();
  if (pending_tutorial) {
    levels::start_tutorial_mode();
    tutorial::start();
  } else if (pending_infinite) {
    tutorial::stop();
    levels::start_infinite_mode();
    const std::string title =
        levels::game_mode() == levels::GameMode::Recipe
            ? "Round " + std::to_string(levels::infinite_round_index + 1)
            : "Daily Infinity";
    begin_started_level_intro(title, levels::game_mode() == levels::GameMode::Recipe);
    return;
  } else {
    tutorial::stop();
    levels::start_level(pending_level_index);
    const std::string title =
        "Level " + std::to_string(pending_level_index + 1) + ": " +
        display_level_name(levels::parsed_levels[pending_level_index].id);
    begin_started_level_intro(title, levels::game_mode() == levels::GameMode::Recipe);
    return;
  }
  player::reset_for_new_level();
  camera_shake::reset();
  vfx::reset_wobble_offsets();
  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->activate();
    main_scene->set_pause(true);
  }
  if (auto* menu_scene = scene::get_scene("menu")) {
    menu_scene->set_pause(true);
  }
  countdown::start("main", 3, []() {
    camera_shake::reset();
    vfx::reset_wobble_offsets();
    if (auto* main_scene = scene::get_scene("main")) {
      main_scene->set_pause(false);
    }
  });
  has_pending_level = false;
  pending_infinite = false;
  pending_tutorial = false;
  suppress_input_for_frames(2);
  set_menu_mode(MenuMode::Main);
}

inline void finish_level_intro_countdown() {
  camera_shake::reset();
  vfx::reset_wobble_offsets();
  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->set_pause(false);
  }
}

inline void begin_started_level_intro(const std::string& title, bool recipe_first) {
  player::reset_for_new_level();
  camera_shake::reset();
  vfx::reset_wobble_offsets();
  if (auto* menu_scene = scene::get_scene("menu")) {
    menu_scene->set_pause(true);
  }
  has_pending_level = false;
  pending_infinite = false;
  pending_tutorial = false;
  suppress_input_for_frames(2);
  set_menu_mode(MenuMode::Main);
  auto done = []() {
    finish_level_intro_countdown();
  };
  if (recipe_first) {
    level_intro::start_recipe_then_countdown(title, levels::mode_label() + " Mode",
                                             std::move(done));
    return;
  }
  level_intro::start(title, levels::mode_label() + " Mode", std::move(done));
}

inline void start_level_from_menu(size_t level_index) {
  if (level_index >= levels::parsed_levels.size()) return;
  tutorial::stop();
  awaiting_name_entry = false;
  show_leaderboard = false;
  text_input::end();
  levels::start_level(level_index);
  const std::string title =
      "Level " + std::to_string(level_index + 1) + ": " +
      display_level_name(levels::parsed_levels[level_index].id);
  begin_started_level_intro(title, levels::game_mode() == levels::GameMode::Recipe);
}

inline void start_infinite_from_menu() {
  tutorial::stop();
  awaiting_name_entry = false;
  show_leaderboard = false;
  text_input::end();
  levels::start_infinite_mode();
  const std::string title =
      levels::game_mode() == levels::GameMode::Recipe
          ? "Round " + std::to_string(levels::infinite_round_index + 1)
          : "Daily Infinity";
  begin_started_level_intro(title, levels::game_mode() == levels::GameMode::Recipe);
}

inline void enter_objective_mode(size_t level_index) {
  if (level_index >= levels::parsed_levels.size()) return;
  awaiting_name_entry = false;
  show_leaderboard = false;
  text_input::end();
  pending_level_index = level_index;
  has_pending_level = true;
  pending_infinite = false;
  pending_tutorial = false;
  refresh_objective_lines(level_index);
  set_menu_mode(MenuMode::Objective);
}

inline void enter_infinite_objective_mode() {
  awaiting_name_entry = false;
  show_leaderboard = false;
  text_input::end();
  pending_level_index = levels::infinite_menu_index();
  has_pending_level = true;
  pending_infinite = true;
  pending_tutorial = false;
  levels::prepare_infinite_preview();
  const std::string label = infinity_objective_label();
  refresh_objective_lines_from_level(levels::infinite_level, label);
  set_menu_mode(MenuMode::Objective);
}

inline void enter_tutorial_objective_mode() {
  awaiting_name_entry = false;
  show_leaderboard = false;
  text_input::end();
  pending_level_index = levels::tutorial_menu_index();
  has_pending_level = true;
  pending_infinite = false;
  pending_tutorial = true;

  update_text(objective_title, "Tutorial");
  update_text(objective_level, "Learn catching and bats, then recipes");
  active_objective_lines = 6;
  const std::array<std::string, 6> lines{
      "1. Move left and right",
      "2. Catch three mushrooms",
      "3. Send three bats",
      "4. Collect far pairs",
      "5. Shoot three mukhomors",
      "6. Complete the recipe",
  };
  for (size_t i = 0; i < lines.size(); ++i) {
    set_line_icon_texture(objective_recipe_lines[i], "");
    update_text(objective_recipe_lines[i], lines[i]);
  }
  for (size_t i = active_objective_lines; i < kMaxLevelLines; ++i) {
    set_line_icon_texture(objective_recipe_lines[i], "");
    update_text(objective_recipe_lines[i], "");
  }
  update_text(objective_hint, "Press Enter or tap to start tutorial");
  set_menu_mode(MenuMode::Objective);
}

inline void enter_game_over_mode() {
  if (!levels::last_result_valid) return;
  awaiting_name_entry = false;
  pending_leaderboard_score = 0;
  if (levels::last_result.infinite_mode) {
    pending_leaderboard_score = levels::last_result.global_score;
    if (leaderboard::qualifies(pending_leaderboard_score)) {
      awaiting_name_entry = true;
      text_input::begin(kNameMaxLength);
    } else {
      text_input::end();
    }
  } else {
    text_input::end();
  }
  refresh_gameover_lines();
  has_pending_level = false;
  pending_infinite = false;
  pending_tutorial = false;
  set_menu_mode(MenuMode::GameOver);
}

inline void enter_main_menu_mode() {
  tutorial::stop();
  awaiting_name_entry = false;
  show_leaderboard = false;
  text_input::end();
  refresh_level_lines();
  refresh_instruction_line();
  refresh_mode_line();
  refresh_settings_line();
  refresh_audio_line();
  refresh_tutorial_line();
  refresh_status_line();
  relayout_main_rows_for_variable_heights();
  has_pending_level = false;
  pending_infinite = false;
  pending_tutorial = false;
  suppress_input_for_frames(2);
  set_menu_mode(MenuMode::Main);
}

inline void enter_settings_mode() {
  pending_control_remap.reset();
  settings_hint_message.clear();
  refresh_settings_lines();
  relayout_settings_rows_for_variable_heights();
  suppress_input_for_frames(1);
  set_menu_mode(MenuMode::Settings);
}

struct MenuController : public dynamic::DynamicObject {
  MenuController() : dynamic::DynamicObject() {}

  static constexpr size_t kCollectorMainSlot = 0;
  static constexpr size_t kRecipeMainSlot = 1;
  static constexpr size_t kTutorialMainSlot = 2;
  static constexpr size_t kSettingsMainSlot = 3;
  static constexpr size_t kLevelMainSlotOffset = 4;
  static constexpr size_t kSettingsVolumeSlot = 0;
  static constexpr size_t kSettingsControlSlotOffset = 1;
  static constexpr size_t kSettingsResetSlot = kSettingsControlSlotOffset + controls::kActionCount;
  static constexpr size_t kSettingsBackSlot = kSettingsResetSlot + 1;
  static constexpr size_t kSettingsSlotCount = kSettingsBackSlot + 1;

  void update_pointer() {
    for (const auto& evt : input::events()) {
      if (evt.kind == engine::InputKind::PointerMove ||
          evt.kind == engine::InputKind::PointerDown) {
        last_pointer = glm::vec2{static_cast<float>(evt.x), static_cast<float>(evt.y)};
      }
    }
  }

  size_t main_slot_count(size_t current_levels) const {
    return kLevelMainSlotOffset + current_levels;
  }

  std::optional<size_t> main_slot_level_index(size_t slot, size_t current_levels) const {
    if (slot < kLevelMainSlotOffset) return std::nullopt;
    const size_t level_index = slot - kLevelMainSlotOffset;
    if (level_index >= current_levels) return std::nullopt;
    return level_index;
  }

  bool is_main_slot_selectable(size_t slot, size_t current_levels) const {
    if (slot == kCollectorMainSlot || slot == kRecipeMainSlot ||
        slot == kTutorialMainSlot || slot == kSettingsMainSlot) {
      return true;
    }
    const auto level_index = main_slot_level_index(slot, current_levels);
    if (!level_index) return false;
    return is_selectable_level(*level_index);
  }

  std::optional<size_t> main_slot_at_point(const glm::vec2& point, size_t current_levels) const {
    if (point_hits_line(mode_line, point)) {
      return kCollectorMainSlot;
    }
    if (point_hits_line(mode_recipe_line, point)) {
      return kRecipeMainSlot;
    }
    if (point_hits_line(tutorial_line, point)) {
      return kTutorialMainSlot;
    }
    if (point_hits_line(settings_line, point)) {
      return kSettingsMainSlot;
    }
    auto level_index = level_index_at_point(point);
    if (!level_index || *level_index >= current_levels) {
      return std::nullopt;
    }
    return *level_index + kLevelMainSlotOffset;
  }

  TextLine* settings_line_for_slot(size_t slot) const {
    if (slot == kSettingsVolumeSlot) return &audio_line;
    if (slot >= kSettingsControlSlotOffset && slot < kSettingsResetSlot) {
      return &settings_control_lines[slot - kSettingsControlSlotOffset];
    }
    if (slot == kSettingsResetSlot) return &settings_reset;
    if (slot == kSettingsBackSlot) return &settings_back;
    return nullptr;
  }

  std::optional<size_t> settings_slot_at_point(const glm::vec2& point) const {
    for (size_t slot = 0; slot < kSettingsSlotCount; ++slot) {
      const TextLine* line = settings_line_for_slot(slot);
      if (line && point_hits_line(*line, point)) return slot;
    }
    return std::nullopt;
  }

  void ensure_main_selection(size_t current_levels) {
    const size_t slot_count = main_slot_count(current_levels);
    if (selected_main_slot && *selected_main_slot < slot_count &&
        is_main_slot_selectable(*selected_main_slot, current_levels)) {
      return;
    }
    const size_t preferred_mode_slot =
        levels::game_mode() == levels::GameMode::Collector ? kCollectorMainSlot : kRecipeMainSlot;
    if (preferred_mode_slot < slot_count &&
        is_main_slot_selectable(preferred_mode_slot, current_levels)) {
      selected_main_slot = preferred_mode_slot;
      return;
    }
    for (size_t slot = 0; slot < slot_count; ++slot) {
      if (!is_main_slot_selectable(slot, current_levels)) continue;
      selected_main_slot = slot;
      return;
    }
    selected_main_slot.reset();
  }

  void move_main_selection(int direction, size_t current_levels) {
    if (direction == 0) return;
    const size_t slot_count = main_slot_count(current_levels);
    if (slot_count == 0) return;
    ensure_main_selection(current_levels);
    if (!selected_main_slot) return;

    const int count = static_cast<int>(slot_count);
    const int start = static_cast<int>(*selected_main_slot);
    for (int step = 1; step <= count; ++step) {
      int candidate = start + direction * step;
      while (candidate < 0) {
        candidate += count;
      }
      candidate %= count;
      if (!is_main_slot_selectable(static_cast<size_t>(candidate), current_levels)) continue;
      selected_main_slot = static_cast<size_t>(candidate);
      return;
    }
  }

  void ensure_settings_selection() {
    if (selected_settings_slot < kSettingsSlotCount) return;
    selected_settings_slot = 0;
  }

  void move_settings_selection(int direction) {
    if (direction == 0) return;
    ensure_settings_selection();
    int next = static_cast<int>(selected_settings_slot) + (direction > 0 ? 1 : -1);
    while (next < 0) {
      next += static_cast<int>(kSettingsSlotCount);
    }
    next %= static_cast<int>(kSettingsSlotCount);
    selected_settings_slot = static_cast<size_t>(next);
  }

  void apply_settings_visuals(std::optional<size_t> hovered_slot) {
    ensure_settings_selection();
    for (size_t slot = 0; slot < kSettingsSlotCount; ++slot) {
      TextLine* line = settings_line_for_slot(slot);
      if (!line) continue;
      const bool selected = slot == selected_settings_slot;
      const bool hovered = hovered_slot && *hovered_slot == slot;
      set_line_visual_state(*line, selected, hovered, !selected);
    }
    for (size_t i = 0; i < kMaxLevelLines; ++i) {
      set_line_visual_state(level_lines[i], false, false, false);
    }
    set_mode_switch_visual_state(false, false, false, false, false, false, false);
    set_line_visual_state(settings_line, false, false, false);
  }

  void set_settings_hint(std::string message) {
    settings_hint_message = std::move(message);
    refresh_settings_lines();
    relayout_settings_rows_for_variable_heights();
  }

  void begin_control_remap(controls::Action action) {
    if (controls::is_mobile_layout()) {
      pending_control_remap.reset();
      set_settings_hint("Controls are read-only on mobile");
      return;
    }
    pending_control_remap = action;
    settings_hint_message =
        "Press a new key for " + std::string(controls::action_label(action));
    refresh_settings_lines();
    relayout_settings_rows_for_variable_heights();
  }

  void complete_control_remap(int key_code) {
    if (!pending_control_remap) return;
    const int key = controls::canonical_key_code(key_code);
    if (!controls::is_supported_key(key)) {
      set_settings_hint("Use letters, numbers, or arrows");
      return;
    }
    if (auto used_by = controls::action_for_key(key);
        used_by && *used_by != *pending_control_remap) {
      set_settings_hint(controls::key_label(key) + " is already used");
      return;
    }
    const controls::Action action = *pending_control_remap;
    if (controls::set_binding(action, key)) {
      pending_control_remap.reset();
      set_settings_hint(std::string(controls::action_label(action)) + " set to " +
                        controls::key_label(key));
    }
  }

  void cancel_control_remap() {
    if (!pending_control_remap) return;
    pending_control_remap.reset();
    set_settings_hint("Remap cancelled");
  }

  void choose_mode(levels::GameMode new_mode) {
    levels::set_game_mode(new_mode);
    refresh_level_lines();
    refresh_instruction_line();
    refresh_mode_line();
    const size_t refreshed_levels = available_levels();
    ensure_main_selection(refreshed_levels);
    update_hover_state(refreshed_levels);
  }

  void handle_selected_settings_action(size_t current_levels) {
    ensure_settings_selection();
    if (selected_settings_slot == kSettingsVolumeSlot) {
      toggle_mute(current_levels);
      refresh_settings_lines();
      relayout_settings_rows_for_variable_heights();
      return;
    }
    if (selected_settings_slot >= kSettingsControlSlotOffset &&
        selected_settings_slot < kSettingsResetSlot) {
      begin_control_remap(
          static_cast<controls::Action>(selected_settings_slot - kSettingsControlSlotOffset));
      return;
    }
    if (selected_settings_slot == kSettingsResetSlot) {
      pending_control_remap.reset();
      controls::reset_to_defaults();
      set_settings_hint("Controls reset");
      return;
    }
    if (selected_settings_slot == kSettingsBackSlot) {
      enter_main_menu_mode();
    }
  }

  void move_gameover_selection(int direction) {
    if (direction == 0) return;
    const size_t count = gameover_action_count();
    if (count == 0) return;
    ensure_gameover_selection();
    int next = static_cast<int>(selected_gameover_index) + direction;
    while (next < 0) {
      next += static_cast<int>(count);
    }
    next %= static_cast<int>(count);
    selected_gameover_index = static_cast<size_t>(next);
  }

  bool is_option_row_slot(size_t slot) const {
    return slot == kCollectorMainSlot || slot == kRecipeMainSlot ||
           slot == kTutorialMainSlot;
  }

  void move_option_row_selection(int direction) {
    if (!selected_main_slot || !is_option_row_slot(*selected_main_slot)) return;
    const int count = 3;
    int next = static_cast<int>(*selected_main_slot) + (direction > 0 ? 1 : -1);
    while (next < 0) {
      next += count;
    }
    next %= count;
    selected_main_slot = static_cast<size_t>(next);
  }

  void toggle_mute(size_t current_levels) {
    shrooms::audio::toggle_muted();
    cached_audio_muted = shrooms::audio::is_muted();
    refresh_audio_line();
    ensure_main_selection(current_levels);
    update_hover_state(current_levels);
  }

  void change_volume(float delta, size_t current_levels) {
    shrooms::audio::set_master_gain(shrooms::audio::master_gain() + delta);
    cached_master_gain = shrooms::audio::master_gain();
    refresh_audio_line();
    ensure_main_selection(current_levels);
    update_hover_state(current_levels);
  }

  void set_volume_from_pointer(const glm::vec2& point, size_t current_levels) {
    if (audio_line.slider_track_size.x <= 0.0f) return;
    const float t = clamp_unit((point.x - audio_line.slider_track_pos.x) / audio_line.slider_track_size.x);
    shrooms::audio::set_master_gain(t);
    cached_master_gain = shrooms::audio::master_gain();
    refresh_audio_line();
    ensure_main_selection(current_levels);
    update_hover_state(current_levels);
  }

  void handle_selected_main_action(size_t slot, size_t current_levels) {
    if (slot == kCollectorMainSlot) {
      choose_mode(levels::GameMode::Collector);
      return;
    }
    if (slot == kRecipeMainSlot) {
      choose_mode(levels::GameMode::Recipe);
      return;
    }
    if (slot == kTutorialMainSlot) {
      enter_tutorial_objective_mode();
      return;
    }
    if (slot == kSettingsMainSlot) {
      enter_settings_mode();
      return;
    }
    const auto level_index = main_slot_level_index(slot, current_levels);
    if (!level_index || !is_selectable_level(*level_index)) return;
    handle_level_selection(*level_index);
  }

  void apply_main_visuals(std::optional<size_t> hovered_slot, size_t current_levels) {
    const bool has_selection = selected_main_slot.has_value();
    for (size_t i = 0; i < kMaxLevelLines; ++i) {
      if (i >= current_levels) {
        set_line_visual_state(level_lines[i], false, false, false);
        continue;
      }
      const size_t slot = i + kLevelMainSlotOffset;
      const bool selectable = is_selectable_level(i);
      const bool selected = selected_main_slot && slot == *selected_main_slot;
      const bool hovered = hovered_slot && slot == *hovered_slot;
      const bool locked = !selectable;
      const bool dimmed = !selected && has_selection;
      set_line_visual_state(level_lines[i], selected, hovered, dimmed, locked);
    }
    const bool collector_selected =
        selected_main_slot && *selected_main_slot == kCollectorMainSlot;
    const bool recipe_selected = selected_main_slot && *selected_main_slot == kRecipeMainSlot;
    const bool tutorial_selected = selected_main_slot && *selected_main_slot == kTutorialMainSlot;
    const bool collector_hovered = hovered_slot && *hovered_slot == kCollectorMainSlot;
    const bool recipe_hovered = hovered_slot && *hovered_slot == kRecipeMainSlot;
    const bool tutorial_hovered = hovered_slot && *hovered_slot == kTutorialMainSlot;
    const bool row_selected = collector_selected || recipe_selected || tutorial_selected;
    const bool row_dimmed = !row_selected && has_selection;
    set_mode_switch_visual_state(collector_selected, recipe_selected, tutorial_selected,
                                 collector_hovered, recipe_hovered, tutorial_hovered,
                                 row_dimmed);
    const bool settings_selected =
        selected_main_slot && *selected_main_slot == kSettingsMainSlot;
    const bool settings_hovered = hovered_slot && *hovered_slot == kSettingsMainSlot;
    set_line_visual_state(settings_line, settings_selected, settings_hovered,
                          !settings_selected && has_selection);
    set_line_visual_state(gameover_restart, false, false, false);
    set_line_visual_state(gameover_main_menu, false, false, false);
    set_line_visual_state(gameover_share, false, false, false);
  }

  bool gameover_has_share() const {
    return levels::last_result_valid && levels::last_result.infinite_mode && !awaiting_name_entry;
  }

  size_t gameover_action_count() const { return gameover_has_share() ? 3 : 2; }

  void ensure_gameover_selection() {
    const size_t count = gameover_action_count();
    if (count == 0) {
      selected_gameover_index = 0;
      return;
    }
    if (selected_gameover_index >= count) {
      selected_gameover_index = 0;
    }
  }

  void apply_gameover_visuals(bool hover_restart, bool hover_menu, bool hover_share) {
    ensure_gameover_selection();
    const bool restart_selected = (selected_gameover_index == 0);
    const bool menu_selected = (selected_gameover_index == 1);
    const bool share_visible = gameover_has_share();
    const bool share_selected = share_visible && (selected_gameover_index == 2);
    set_line_visual_state(gameover_restart, restart_selected, hover_restart, !restart_selected);
    set_line_visual_state(gameover_main_menu, menu_selected, hover_menu, !menu_selected);
    if (share_visible) {
      set_line_visual_state(gameover_share, share_selected, hover_share, !share_selected);
    } else {
      set_line_visual_state(gameover_share, false, false, false);
    }
    for (size_t i = 0; i < kMaxLevelLines; ++i) {
      set_line_visual_state(level_lines[i], false, false, false);
    }
    set_mode_switch_visual_state(false, false, false, false, false, false, false);
    set_line_visual_state(settings_line, false, false, false);
    set_line_visual_state(audio_line, false, false, false);
    for (auto& line : settings_control_lines) {
      set_line_visual_state(line, false, false, false);
    }
    set_line_visual_state(settings_reset, false, false, false);
    set_line_visual_state(settings_back, false, false, false);
  }

  void clear_hover(size_t current_levels) {
    for (size_t i = 0; i < kMaxLevelLines; ++i) {
      set_line_hovered(objective_recipe_lines[i], false);
    }
    if (menu_mode == MenuMode::Main) {
      apply_main_visuals(std::nullopt, current_levels);
    } else if (menu_mode == MenuMode::Settings) {
      apply_settings_visuals(std::nullopt);
    } else {
      for (size_t i = 0; i < kMaxLevelLines; ++i) {
        set_line_visual_state(level_lines[i], false, false, false);
      }
      set_mode_switch_visual_state(false, false, false, false, false, false, false);
      set_line_visual_state(settings_line, false, false, false);
      set_line_visual_state(audio_line, false, false, false);
      for (auto& line : settings_control_lines) {
        set_line_visual_state(line, false, false, false);
      }
      set_line_visual_state(settings_reset, false, false, false);
      set_line_visual_state(settings_back, false, false, false);
    }
    if (menu_mode == MenuMode::GameOver && !awaiting_name_entry) {
      apply_gameover_visuals(false, false, false);
    } else {
      set_line_visual_state(gameover_restart, false, false, false);
      set_line_visual_state(gameover_main_menu, false, false, false);
      set_line_visual_state(gameover_share, false, false, false);
    }
  }

  void update_hover_state(size_t current_levels) {
    if (menu_mode == MenuMode::Main) {
      ensure_main_selection(current_levels);
      std::optional<size_t> hovered_slot;
      if (last_pointer) {
        hovered_slot = main_slot_at_point(*last_pointer, current_levels);
      }
      if (hovered_slot && !is_main_slot_selectable(*hovered_slot, current_levels)) {
        hovered_slot.reset();
      }
      if (hovered_slot) {
        selected_main_slot = *hovered_slot;
      }
      apply_main_visuals(hovered_slot, current_levels);
      return;
    }

    if (menu_mode == MenuMode::GameOver) {
      if (awaiting_name_entry) {
        for (size_t i = 0; i < kMaxLevelLines; ++i) {
          set_line_visual_state(level_lines[i], false, false, false);
        }
        set_line_visual_state(gameover_restart, false, false, false);
        set_line_visual_state(gameover_main_menu, false, false, false);
        set_line_visual_state(gameover_share, false, false, false);
        set_line_visual_state(settings_line, false, false, false);
        set_line_visual_state(audio_line, false, false, false);
        for (auto& line : settings_control_lines) {
          set_line_visual_state(line, false, false, false);
        }
        set_line_visual_state(settings_reset, false, false, false);
        set_line_visual_state(settings_back, false, false, false);
        return;
      }
      bool hover_restart = false;
      bool hover_menu = false;
      bool hover_share = false;
      if (last_pointer) {
        hover_restart = point_hits_line(gameover_restart, *last_pointer);
        hover_menu = point_hits_line(gameover_main_menu, *last_pointer);
        hover_share = gameover_has_share() && point_hits_line(gameover_share, *last_pointer);
      }
      if (hover_restart) {
        selected_gameover_index = 0;
      } else if (hover_menu) {
        selected_gameover_index = 1;
      } else if (hover_share) {
        selected_gameover_index = 2;
      }
      apply_gameover_visuals(hover_restart, hover_menu, hover_share);
      return;
    }

    if (menu_mode == MenuMode::Settings) {
      ensure_settings_selection();
      std::optional<size_t> hovered_slot;
      if (last_pointer) {
        hovered_slot = settings_slot_at_point(*last_pointer);
      }
      if (hovered_slot) {
        selected_settings_slot = *hovered_slot;
      }
      apply_settings_visuals(hovered_slot);
      return;
    }

    clear_hover(current_levels);
  }

  void handle_selected_gameover_action(size_t restart_index) {
    ensure_gameover_selection();
    if (selected_gameover_index == 0) {
      if (levels::last_result.infinite_mode) {
        start_infinite_from_menu();
      } else if (levels::last_result.tutorial_mode) {
        enter_tutorial_objective_mode();
      } else {
        start_level_from_menu(restart_index);
      }
      return;
    }
    if (selected_gameover_index == 2 && gameover_has_share()) {
      const std::string text = build_share_text(levels::last_result.global_score);
      const share_bridge::ShareRoute route = share_bridge::share_or_copy_score_text(text);
      switch (route) {
        case share_bridge::ShareRoute::WebShare:
          update_text(gameover_hint, "Opened share menu");
          break;
        case share_bridge::ShareRoute::Clipboard:
          update_text(gameover_hint, "Copied to clipboard");
          break;
        case share_bridge::ShareRoute::Unsupported:
        default:
          update_text(gameover_hint, "Share unavailable");
          break;
      }
      set_menu_mode(MenuMode::GameOver);
      return;
    }
    enter_main_menu_mode();
  }

  void update() override {
    const size_t current_levels = available_levels();
    if (cached_game_mode != levels::game_mode()) {
      cached_game_mode = levels::game_mode();
      refresh_level_lines();
      refresh_instruction_line();
      refresh_mode_line();
      ensure_main_selection(current_levels);
    }

    if (cached_level_count != current_levels) {
      cached_level_count = current_levels;
      refresh_level_lines();
      refresh_instruction_line();
      ensure_main_selection(current_levels);
    } else if (cached_last_played != levels::last_played_level_index) {
      cached_last_played = levels::last_played_level_index;
      refresh_level_lines();
    }

    if (cached_unlocked != levels::unlocked_levels()) {
      cached_unlocked = levels::unlocked_levels();
      refresh_level_lines();
      refresh_instruction_line();
      ensure_main_selection(current_levels);
    }

    if (cached_status != levels::last_game_status) {
      cached_status = levels::last_game_status;
      refresh_status_line();
    }

    if (cached_audio_muted != shrooms::audio::is_muted()) {
      cached_audio_muted = shrooms::audio::is_muted();
      refresh_audio_line();
    }
    if (cached_master_gain != shrooms::audio::master_gain()) {
      cached_master_gain = shrooms::audio::master_gain();
      refresh_audio_line();
    }

    if (menu_mode != previous_mode) {
      if (menu_mode == MenuMode::Main) {
        ensure_main_selection(current_levels);
      } else if (menu_mode == MenuMode::Settings) {
        selected_settings_slot = kSettingsVolumeSlot;
        ensure_settings_selection();
      } else if (menu_mode == MenuMode::GameOver && !awaiting_name_entry) {
        selected_gameover_index = 0;
        ensure_gameover_selection();
      }
      previous_mode = menu_mode;
    }

    if (levels::level_finished && levels::last_result_valid) {
      if (!completion_acknowledged) {
        if (shrooms::scenes::menu) {
          shrooms::scenes::menu->activate();
          shrooms::scenes::menu->set_pause(true);
        }
        if (shrooms::scenes::main) {
          shrooms::scenes::main->set_pause(true);
        }
        player::reset_for_new_level();
        if (levels::last_result.infinite_mode) {
          enter_game_over_mode();
        } else {
          enter_main_menu_mode();
        }
        completion_acknowledged = true;
      }
    } else {
      completion_acknowledged = false;
    }

    update_pointer();
    if (!shrooms::scenes::menu || !shrooms::scenes::menu->is_active) {
      dragging_audio_slider = false;
      clear_hover(current_levels);
      return;
    }
    if (block_input_frames > 0) {
      dragging_audio_slider = false;
      block_input_frames--;
      clear_hover(current_levels);
      return;
    }
    update_hover_state(current_levels);

    if (menu_mode == MenuMode::Main) {
      ensure_main_selection(current_levels);
      for (const auto& evt : input::events()) {
        if (evt.kind == engine::InputKind::PointerUp) {
          dragging_audio_slider = false;
        }
      }
      for (size_t i = 0; i < kMaxLevelLines; ++i) {
        const int key_code = (i == 9) ? '0' : static_cast<int>('1' + i);
        const bool pressed = input::get_button_state(key_code);
        if (pressed && !previous_state[i] && i < current_levels && is_selectable_level(i)) {
          previous_state[i] = pressed;
          handle_level_selection(i);
          return;
        }
        previous_state[i] = pressed;
      }

      for (const auto& evt : input::events()) {
        if (evt.kind != engine::InputKind::KeyDown) continue;
        const int key = input::normalize_key_code(evt.key_code);
        if (key == 'T') {
          enter_tutorial_objective_mode();
          return;
        }
        if (key == 'V') {
          enter_settings_mode();
          return;
        }
        if (is_arrow_up_key(key)) {
          move_main_selection(-1, current_levels);
          update_hover_state(current_levels);
          continue;
        }
        if (is_arrow_down_key(key)) {
          move_main_selection(1, current_levels);
          update_hover_state(current_levels);
          continue;
        }
        if (selected_main_slot && is_option_row_slot(*selected_main_slot) &&
            is_arrow_left_key(key)) {
          move_option_row_selection(-1);
          update_hover_state(current_levels);
          continue;
        }
        if (selected_main_slot && is_option_row_slot(*selected_main_slot) &&
            is_arrow_right_key(key)) {
          move_option_row_selection(1);
          update_hover_state(current_levels);
          continue;
        }
        if (is_confirm_key(key) && selected_main_slot) {
          handle_selected_main_action(*selected_main_slot, current_levels);
          return;
        }
      }

      for (const auto& evt : input::events()) {
        if (evt.kind != engine::InputKind::PointerDown) continue;
        const glm::vec2 point{static_cast<float>(evt.x), static_cast<float>(evt.y)};
        auto slot = main_slot_at_point(point, current_levels);
        if (slot && is_main_slot_selectable(*slot, current_levels)) {
          selected_main_slot = *slot;
          handle_selected_main_action(*slot, current_levels);
          return;
        }
      }
      return;
    }

    if (menu_mode == MenuMode::Settings) {
      if (controls::is_mobile_layout() && pending_control_remap) {
        pending_control_remap.reset();
        set_settings_hint("Controls are read-only on mobile");
      }
      for (const auto& evt : input::events()) {
        if (evt.kind == engine::InputKind::PointerUp) {
          dragging_audio_slider = false;
        }
      }

      if (pending_control_remap) {
        for (const auto& evt : input::events()) {
          if (evt.kind != engine::InputKind::KeyDown) continue;
          const int key = controls::canonical_key_code(evt.key_code);
          if (key == 27) {
            cancel_control_remap();
            return;
          }
          complete_control_remap(key);
          return;
        }
        return;
      }

      if (dragging_audio_slider) {
        for (const auto& evt : input::events()) {
          if (evt.kind != engine::InputKind::PointerMove &&
              evt.kind != engine::InputKind::PointerDown) {
            continue;
          }
          set_volume_from_pointer(glm::vec2{static_cast<float>(evt.x), static_cast<float>(evt.y)},
                                  current_levels);
          return;
        }
      }

      for (const auto& evt : input::events()) {
        if (evt.kind == engine::InputKind::PointerDown) {
          const glm::vec2 point{static_cast<float>(evt.x), static_cast<float>(evt.y)};
          if (point_hits_slider(audio_line, point)) {
            selected_settings_slot = kSettingsVolumeSlot;
            dragging_audio_slider = true;
            set_volume_from_pointer(point, current_levels);
            return;
          }
          auto slot = settings_slot_at_point(point);
          if (slot) {
            selected_settings_slot = *slot;
            handle_selected_settings_action(current_levels);
            return;
          }
          continue;
        }

        if (evt.kind != engine::InputKind::KeyDown) continue;
        const int key = controls::canonical_key_code(evt.key_code);
        if (is_arrow_up_key(key)) {
          move_settings_selection(-1);
          update_hover_state(current_levels);
          continue;
        }
        if (is_arrow_down_key(key)) {
          move_settings_selection(1);
          update_hover_state(current_levels);
          continue;
        }
        if ((key == 'A' || is_arrow_left_key(key)) &&
            selected_settings_slot == kSettingsVolumeSlot) {
          change_volume(-kVolumeAdjustStep, current_levels);
          refresh_settings_lines();
          return;
        }
        if ((key == 'D' || is_arrow_right_key(key)) &&
            selected_settings_slot == kSettingsVolumeSlot) {
          change_volume(kVolumeAdjustStep, current_levels);
          refresh_settings_lines();
          return;
        }
        if (key == 27) {
          enter_main_menu_mode();
          return;
        }
        if (is_confirm_key(key)) {
          handle_selected_settings_action(current_levels);
          return;
        }
      }
      return;
    }

    if (menu_mode == MenuMode::Objective) {
      dragging_audio_slider = false;
      for (const auto& evt : input::events()) {
        if (evt.kind == engine::InputKind::PointerDown) {
          start_pending_level();
          return;
        }
        if (evt.kind != engine::InputKind::KeyDown) continue;
        const int key = input::normalize_key_code(evt.key_code);
        if (is_confirm_key(key)) {
          start_pending_level();
          return;
        }
      }
      return;
    }

    if (menu_mode == MenuMode::GameOver) {
      dragging_audio_slider = false;
      if (!levels::last_result_valid) {
        enter_main_menu_mode();
        return;
      }
      if (awaiting_name_entry) {
        text_input::update(input::events());
        refresh_name_entry_lines();
        if (text_input::consume_submitted()) {
          leaderboard::insert(text_input::value(), pending_leaderboard_score);
          awaiting_name_entry = false;
          refresh_leaderboard_lines();
          refresh_gameover_lines();
          set_menu_mode(MenuMode::GameOver);
          return;
        }
        for (const auto& evt : input::events()) {
          if (evt.kind != engine::InputKind::PointerDown) continue;
          const glm::vec2 point{static_cast<float>(evt.x), static_cast<float>(evt.y)};
          if (point_hits_line(gameover_name_prompt, point) ||
              point_hits_line(gameover_name_value, point)) {
            text_input::begin(kNameMaxLength, text_input::value());
            break;
          }
        }
        return;
      }
      const size_t restart_index = levels::last_result.level_index;
      for (const auto& evt : input::events()) {
        if (evt.kind == engine::InputKind::PointerDown) {
          const glm::vec2 point{static_cast<float>(evt.x), static_cast<float>(evt.y)};
          if (point_hits_line(gameover_restart, point)) {
            selected_gameover_index = 0;
            handle_selected_gameover_action(restart_index);
            return;
          }
          if (point_hits_line(gameover_main_menu, point)) {
            selected_gameover_index = 1;
            handle_selected_gameover_action(restart_index);
            return;
          }
          if (gameover_has_share() && point_hits_line(gameover_share, point)) {
            selected_gameover_index = 2;
            handle_selected_gameover_action(restart_index);
            return;
          }
          continue;
        }
        if (evt.kind != engine::InputKind::KeyDown) continue;
        const int key = input::normalize_key_code(evt.key_code);
        if (is_arrow_up_key(key)) {
          move_gameover_selection(-1);
          update_hover_state(current_levels);
          continue;
        }
        if (is_arrow_down_key(key)) {
          move_gameover_selection(1);
          update_hover_state(current_levels);
          continue;
        }
        if (key == 'R') {
          selected_gameover_index = 0;
          handle_selected_gameover_action(restart_index);
          return;
        }
        if (key == 'M') {
          selected_gameover_index = 1;
          handle_selected_gameover_action(restart_index);
          return;
        }
        if (key == 'S' && gameover_has_share()) {
          selected_gameover_index = 2;
          handle_selected_gameover_action(restart_index);
          return;
        }
        if (key == 27) {
          selected_gameover_index = 1;
          handle_selected_gameover_action(restart_index);
          return;
        }
        if (is_confirm_key(key)) {
          handle_selected_gameover_action(restart_index);
          return;
        }
      }
    }
  }

  std::array<bool, kMaxLevelLines> previous_state{};
  size_t cached_level_count = std::numeric_limits<size_t>::max();
  size_t cached_last_played = std::numeric_limits<size_t>::max();
  size_t cached_unlocked = std::numeric_limits<size_t>::max();
  levels::GameMode cached_game_mode = levels::GameMode::Collector;
  std::string cached_status;
  bool cached_audio_muted = shrooms::audio::is_muted();
  float cached_master_gain = shrooms::audio::master_gain();
  bool completion_acknowledged = false;
  bool dragging_audio_slider = false;
  std::optional<glm::vec2> last_pointer;
  std::optional<size_t> selected_main_slot;
  size_t selected_settings_slot = 0;
  size_t selected_gameover_index = 0;
  MenuMode previous_mode = MenuMode::Main;
};

inline MenuController menu_controller{};

inline TextLine make_text_line(glm::vec2 position_norm, float font_px, int layer) {
  TextLine line{};
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos =
      shrooms::screen::norm_to_pixels(position_norm + glm::vec2{0.0f, kMenuTextYOffsetNorm});
  auto* text_obj = arena::create<text::TextObject>("", font_px);

  entity->add(transform);
  entity->add(text_obj);
  entity->add(arena::create<layers::ConstLayer>(layer + 1));
  line.text_hidden = arena::create<hidden::HiddenObject>();
  entity->add(line.text_hidden);
  entity->add(arena::create<scene::SceneObject>("menu"));

  line.entity = entity;
  line.transform = transform;
  line.anchor_pos = transform->pos;
  line.text_object = text_obj;
  line.font_px = font_px;
  line.text_color = arena::create<LineTextColor>(line.base_text_color);
  entity->add(line.text_color);

  line.button_entity = arena::create<ecs::Entity>();
  line.button_transform = arena::create<transform::NoRotationTransform>();
  line.button_transform->pos = transform->pos;
  line.button_entity->add(line.button_transform);
  line.button_entity->add(arena::create<layers::ConstLayer>(layer - 1));
  line.button_quad = arena::create<render_system::QuadRenderable>(
      0.0f, 0.0f, engine::UIColor{0.0f, 0.0f, 0.0f, 0.0f});
  line.button_entity->add(line.button_quad);
  line.button_hidden = arena::create<hidden::HiddenObject>();
  line.button_entity->add(line.button_hidden);
  line.button_entity->add(arena::create<scene::SceneObject>("menu"));

  update_text(line, "");
  return line;
}

inline void init() {
  const glm::vec2 view_size{
      static_cast<float>(shrooms::screen::view_width),
      static_cast<float>(shrooms::screen::view_height),
  };

  menu_background = arena::create<ecs::Entity>();
  const float bg_side = std::min(view_size.x, view_size.y);
  const glm::vec2 bg_size{bg_side, bg_side};
  auto* bg_transform = arena::create<transform::NoRotationTransform>();
  bg_transform->pos = glm::vec2{(view_size.x - bg_side) * 0.5f, (view_size.y - bg_side) * 0.5f};
  menu_background->add(bg_transform);
  menu_background->add(arena::create<layers::ConstLayer>(-2));
  const engine::TextureId bg_tex = engine::resources::register_texture(kDefaultMenuBackgroundTexture);
  menu_background_sprite = arena::create<render_system::SpriteRenderable>(bg_tex, bg_size);
  menu_background->add(menu_background_sprite);
  menu_background_transform = bg_transform;
  menu_background->add(arena::create<scene::SceneObject>("menu"));

  status_line = make_text_line(glm::vec2{kMenuTextX, 0.85f}, 22.0f, 6);
  instruction_line = make_text_line(glm::vec2{kMenuTextX, 0.71f}, 20.0f, 6);
  mode_line = make_text_line(glm::vec2{kModeSwitchCenterX, kModeSwitchCenterY}, 18.0f, 6);
  mode_recipe_line = make_text_line(glm::vec2{kModeSwitchCenterX, kModeSwitchCenterY}, 18.0f, 6);
  settings_line = make_text_line(glm::vec2{kMenuTextX, 0.48f}, 19.0f, 6);
  audio_line = make_text_line(glm::vec2{kMenuTextX, 0.42f}, 19.0f, 6);
  tutorial_line = make_text_line(glm::vec2{kMenuTextX, 0.28f}, 20.0f, 6);
  credits_line = make_text_line(glm::vec2{kMenuTextX, -0.92f}, 18.0f, 6);
  update_text(credits_line, "Game by KiK0S, art by deadmarla.");

  auto style_menu_action = [](TextLine& line) {
    line.base_button_color = engine::UIColor{0.15f, 0.15f, 0.15f, 0.9f};
    line.hover_button_color = engine::UIColor{0.28f, 0.2f, 0.35f, 0.98f};
    line.selected_button_color = line.base_button_color;
    line.dimmed_button_color = engine::UIColor{0.08f, 0.08f, 0.08f, 0.5f};
    line.slider_track_color = engine::UIColor{0.92f, 0.94f, 0.98f, 1.0f};
    line.slider_fill_color = engine::UIColor{0.18f, 0.78f, 1.0f, 1.0f};
    line.slider_knob_color = engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f};
    line.slider_dimmed_color = engine::UIColor{0.72f, 0.74f, 0.8f, 1.0f};
    line.selected_text_color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    line.dimmed_text_color = glm::vec4{0.72f, 0.72f, 0.72f, 0.92f};
    line.selected_scale = 1.0f;
  };
  style_menu_action(mode_line);
  style_menu_action(mode_recipe_line);
  style_menu_action(settings_line);
  style_menu_action(audio_line);
  style_menu_action(tutorial_line);
  ensure_line_slider(audio_line);
  set_line_slider_value(audio_line, shrooms::audio::volume_slider_value());

  const glm::vec2 base = glm::vec2{kMenuTextX, 0.25f};
  const float spacing = 0.11f;
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    const glm::vec2 pos = base - glm::vec2(0.0f, spacing * static_cast<float>(i));
    level_lines[i] = make_text_line(pos, 20.0f, 6);
    style_menu_action(level_lines[i]);
  }

  objective_title = make_text_line(glm::vec2{kMenuTextX, 0.65f}, 24.0f, 6);
  objective_level = make_text_line(glm::vec2{kMenuTextX, 0.48f}, 20.0f, 6);
  const glm::vec2 objective_base = glm::vec2{kMenuTextX, 0.25f};
  for (size_t i = 0; i < kMaxLevelLines; ++i) {
    const glm::vec2 pos = objective_base - glm::vec2(0.0f, spacing * static_cast<float>(i));
    objective_recipe_lines[i] = make_text_line(pos, 20.0f, 6);
    objective_recipe_lines[i].icon_before_text = true;
    objective_recipe_lines[i].icon_gap_px = 10.0f;
  }
  objective_hint = make_text_line(glm::vec2{kMenuTextX, -0.8f}, 18.0f, 6);

  settings_title = make_text_line(glm::vec2{kMenuTextX, 0.66f}, 24.0f, 6);
  settings_hint = make_text_line(glm::vec2{kMenuTextX, 0.54f}, 16.0f, 6);
  const glm::vec2 settings_base = glm::vec2{kMenuTextX, 0.24f};
  for (size_t i = 0; i < settings_control_lines.size(); ++i) {
    const glm::vec2 pos = settings_base - glm::vec2(0.0f, spacing * static_cast<float>(i));
    settings_control_lines[i] = make_text_line(pos, 19.0f, 6);
    style_menu_action(settings_control_lines[i]);
  }
  settings_reset = make_text_line(glm::vec2{kMenuTextX, -0.34f}, 19.0f, 6);
  settings_back = make_text_line(glm::vec2{kMenuTextX, -0.48f}, 19.0f, 6);
  style_menu_action(settings_reset);
  style_menu_action(settings_back);

  gameover_title = make_text_line(glm::vec2{kMenuTextX, 0.6f}, 24.0f, 6);
  gameover_level = make_text_line(glm::vec2{kMenuTextX, 0.44f}, 20.0f, 6);
  gameover_collected = make_text_line(glm::vec2{kMenuTextX, 0.28f}, 20.0f, 6);
  gameover_sorted = make_text_line(glm::vec2{kMenuTextX, 0.14f}, 20.0f, 6);
  gameover_score = make_text_line(glm::vec2{kMenuTextX, 0.78f}, 22.0f, 6);
  gameover_name_prompt = make_text_line(glm::vec2{kMenuTextX, -0.42f}, 18.0f, 6);
  gameover_name_value = make_text_line(glm::vec2{kMenuTextX, -0.58f}, 20.0f, 6);
  gameover_restart = make_text_line(glm::vec2{kMenuTextX, -0.42f}, 20.0f, 6);
  gameover_main_menu = make_text_line(glm::vec2{kMenuTextX, -0.58f}, 20.0f, 6);
  gameover_share = make_text_line(glm::vec2{kMenuTextX, -0.74f}, 20.0f, 6);
  gameover_hint = make_text_line(glm::vec2{kMenuTextX, -0.88f}, 18.0f, 6);

  leaderboard_title = make_text_line(glm::vec2{kLeaderboardTextX, 0.62f}, 22.0f, 6);
  const glm::vec2 leaderboard_base = glm::vec2{kLeaderboardTextX, 0.45f};
  const float leaderboard_spacing = 0.1f;
  for (size_t i = 0; i < kLeaderboardLines; ++i) {
    const glm::vec2 pos =
        leaderboard_base - glm::vec2(0.0f, leaderboard_spacing * static_cast<float>(i));
    leaderboard_lines[i] = make_text_line(pos, 18.0f, 6);
  }

  if (gameover_restart.button_quad) {
    gameover_restart.base_button_color = engine::UIColor{0.15f, 0.15f, 0.15f, 0.95f};
    gameover_restart.hover_button_color = engine::UIColor{0.28f, 0.2f, 0.35f, 0.98f};
    gameover_restart.selected_button_color = gameover_restart.base_button_color;
    gameover_restart.dimmed_button_color = engine::UIColor{0.08f, 0.08f, 0.08f, 0.52f};
    gameover_restart.selected_text_color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    gameover_restart.dimmed_text_color = glm::vec4{0.72f, 0.72f, 0.72f, 0.92f};
    gameover_restart.selected_scale = 1.0f;
    gameover_restart.button_quad->color = gameover_restart.base_button_color;
  }
  if (gameover_main_menu.button_quad) {
    gameover_main_menu.base_button_color = engine::UIColor{0.15f, 0.15f, 0.15f, 0.95f};
    gameover_main_menu.hover_button_color = engine::UIColor{0.28f, 0.2f, 0.35f, 0.98f};
    gameover_main_menu.selected_button_color = gameover_main_menu.base_button_color;
    gameover_main_menu.dimmed_button_color = engine::UIColor{0.08f, 0.08f, 0.08f, 0.52f};
    gameover_main_menu.selected_text_color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    gameover_main_menu.dimmed_text_color = glm::vec4{0.72f, 0.72f, 0.72f, 0.92f};
    gameover_main_menu.selected_scale = 1.0f;
    gameover_main_menu.button_quad->color = gameover_main_menu.base_button_color;
  }
  if (gameover_share.button_quad) {
    gameover_share.base_button_color = engine::UIColor{0.15f, 0.15f, 0.15f, 0.95f};
    gameover_share.hover_button_color = engine::UIColor{0.28f, 0.2f, 0.35f, 0.98f};
    gameover_share.selected_button_color = gameover_share.base_button_color;
    gameover_share.dimmed_button_color = engine::UIColor{0.08f, 0.08f, 0.08f, 0.52f};
    gameover_share.selected_text_color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    gameover_share.dimmed_text_color = glm::vec4{0.72f, 0.72f, 0.72f, 0.92f};
    gameover_share.selected_scale = 1.0f;
    gameover_share.button_quad->color = gameover_share.base_button_color;
  }

  refresh_status_line();
  refresh_instruction_line();
  refresh_level_lines();
  refresh_leaderboard_lines();
  refresh_name_entry_lines();

  enter_main_menu_mode();
  if (!levels::has_completed_tutorial()) {
    enter_tutorial_objective_mode();
    start_pending_level();
  }
}

}  // namespace menu
