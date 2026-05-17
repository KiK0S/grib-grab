#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"
#include "glm/glm/geometric.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/color/color_system.hpp"
#include "systems/defer/deferred_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/hidden/hidden_object.hpp"
#include "systems/input/input_system.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/collision/collider_object.hpp"
#include "systems/geometry/shapes/quad.hpp"
#include "systems/render/render_system.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"

#include "level_manager.hpp"
#include "controls.hpp"
#include "countdown.hpp"
#include "player.hpp"
#include "score_hud.hpp"
#include "scoreboard.hpp"
#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"
#include "touchscreen.hpp"

namespace tutorial {

enum class Stage {
  None,
  MoveLeft,
  MoveRight,
  CatchPractice,
  PlaceThreeTraps,
  TrapCollectDemo,
  TrapPairIntro,
  TrapPairPractice,
  ShootPractice,
  RecipeIntro,
  RecipeScenario,
  Complete,
};

inline bool active = false;
inline Stage stage = Stage::None;
inline std::string good_mushroom_type = "borovik";
inline std::string bad_mushroom_type = "mukhomor";
inline float stage_timer = 0.0f;
inline bool stage_restart_pending = false;
inline Stage pending_restart_stage = Stage::None;
inline Stage pending_restart_origin_stage = Stage::None;
inline std::string pending_restart_feedback{};

inline ecs::Entity* title_entity = nullptr;
inline text::TextObject* title_text = nullptr;
inline transform::NoRotationTransform* title_transform = nullptr;
inline hidden::HiddenObject* title_hidden = nullptr;

inline ecs::Entity* hint_entity = nullptr;
inline text::TextObject* hint_text = nullptr;
inline transform::NoRotationTransform* hint_transform = nullptr;
inline hidden::HiddenObject* hint_hidden = nullptr;
inline color::OneColor* hint_color = nullptr;

inline ecs::Entity* marker_entity = nullptr;
inline transform::NoRotationTransform* marker_transform = nullptr;
inline hidden::HiddenObject* marker_hidden = nullptr;
inline render_system::CircleRenderable* marker_circle = nullptr;
inline float marker_radius = 0.0f;

inline constexpr int kTrapTargetCount = 3;
inline std::array<ecs::Entity*, kTrapTargetCount> trap_marker_entities{};
inline std::array<transform::NoRotationTransform*, kTrapTargetCount> trap_marker_transforms{};
inline std::array<hidden::HiddenObject*, kTrapTargetCount> trap_marker_hidden{};
inline std::array<render_system::CircleRenderable*, kTrapTargetCount> trap_marker_circles{};
inline std::array<glm::vec2, kTrapTargetCount> trap_target_centers_px{};
inline std::array<glm::vec2, kTrapTargetCount> trap_familiar_centers_px{};
inline std::array<bool, kTrapTargetCount> trap_target_filled{};
inline int trap_target_filled_count = 0;

inline ecs::Entity* stage_entity_a = nullptr;
inline ecs::Entity* stage_entity_b = nullptr;
inline ecs::Entity* stage_entity_c = nullptr;
inline bool stage_a_done = false;
inline bool stage_b_done = false;
inline bool stage_c_done = false;
inline bool recipe_bad_removed = false;
inline int recipe_good_catches = 0;
inline int catch_practice_catches = 0;
inline int catch_spawn_index = 0;
inline int trap_demo_catches = 0;
inline bool trap_demo_spawned = false;
inline glm::vec2 trap_border_marker_px{0.0f, 0.0f};
inline int pair_practice_pairs_completed = 0;
inline int pair_spawn_index = 0;
inline int pair_current_catches = 0;
inline int shoot_practice_shots = 0;
inline int shoot_spawn_index = 0;
inline bool pair_practice_countdown_started = false;
inline bool recipe_intro_countdown_started = false;
inline bool recipe_bad_shot = false;
inline std::array<ecs::Entity*, 3> recipe_preview_entities{};

inline constexpr int kPracticeTarget = 3;
inline constexpr int kRecipeGoodTarget = 2;
inline constexpr int kRecipeBadTarget = 0;
inline constexpr float kRecipePreviewSeconds = 2.0f;

inline constexpr glm::vec2 kTitleCenterNorm = glm::vec2{0.0f, 0.16f};
inline constexpr glm::vec2 kHintCenterNorm = glm::vec2{0.0f, -0.02f};
inline constexpr float kMaxTextWidthRatio = 0.86f;

inline float view_width() { return static_cast<float>(shrooms::screen::view_width); }
inline float view_height() { return static_cast<float>(shrooms::screen::view_height); }

inline float title_font_px() { return controls::is_mobile_layout() ? 30.0f : 26.0f; }

inline float hint_font_px() { return controls::is_mobile_layout() ? 23.0f : 20.0f; }

inline float player_center_x() {
  if (!player::player_transform) return view_width() * 0.5f;
  return player::player_transform->pos.x + player::player_size.x * 0.5f;
}

inline glm::vec2 player_center_px() {
  if (!player::player_transform) {
    return glm::vec2{view_width() * 0.5f, view_height() * 0.82f};
  }
  return player::player_transform->pos + player::player_size * 0.5f;
}

inline std::string trim_copy(const std::string& value) {
  size_t first = 0;
  while (first < value.size() &&
         std::isspace(static_cast<unsigned char>(value[first])) != 0) {
    ++first;
  }
  size_t last = value.size();
  while (last > first &&
         std::isspace(static_cast<unsigned char>(value[last - 1])) != 0) {
    --last;
  }
  return value.substr(first, last - first);
}

inline bool is_alpha_char(char ch) {
  return std::isalpha(static_cast<unsigned char>(ch)) != 0;
}

inline int break_priority(char ch) {
  switch (ch) {
    case '.':
      return 5;
    case '!':
    case '?':
      return 4;
    case ';':
    case ':':
      return 3;
    case ',':
      return 2;
    default:
      return is_alpha_char(ch) ? -1 : 1;
  }
}

inline float text_width_px(const std::string& value, float font_px) {
  return engine::text::layout_text(value, 0.0f, 0.0f, font_px).width;
}

inline std::string join_lines(const std::vector<std::string>& lines) {
  if (lines.empty()) return "";
  std::string out = lines.front();
  for (size_t i = 1; i < lines.size(); ++i) {
    out += '\n';
    out += lines[i];
  }
  return out;
}

inline std::string wrap_text_for_view(const std::string& value, float font_px,
                                      float max_width_px) {
  if (value.empty()) return value;
  if (text_width_px(value, font_px) <= max_width_px) return value;

  std::vector<std::string> lines{};
  size_t start = 0;
  while (start < value.size()) {
    while (start < value.size() &&
           std::isspace(static_cast<unsigned char>(value[start])) != 0) {
      ++start;
    }
    if (start >= value.size()) break;

    const std::string remaining = value.substr(start);
    if (text_width_px(remaining, font_px) <= max_width_px) {
      const std::string tail = trim_copy(remaining);
      if (!tail.empty()) lines.push_back(tail);
      break;
    }

    size_t best_break = std::string::npos;
    int best_priority = -1;
    for (size_t i = start; i < value.size(); ++i) {
      const char ch = value[i];
      if (is_alpha_char(ch)) continue;
      const std::string candidate = value.substr(start, i - start + 1);
      if (text_width_px(candidate, font_px) > max_width_px) break;
      const int priority = break_priority(ch);
      if (priority > best_priority ||
          (priority == best_priority &&
           (best_break == std::string::npos || i > best_break))) {
        best_break = i;
        best_priority = priority;
      }
    }

    if (best_break == std::string::npos) {
      size_t hard_break = start;
      for (size_t i = start; i < value.size(); ++i) {
        const std::string candidate = value.substr(start, i - start + 1);
        if (text_width_px(candidate, font_px) > max_width_px) break;
        hard_break = i;
      }
      if (hard_break == start && start + 1 < value.size()) {
        hard_break = start + 1;
      }
      best_break = hard_break;
    }

    const std::string line = trim_copy(value.substr(start, best_break - start + 1));
    if (!line.empty()) lines.push_back(line);
    start = best_break + 1;
  }

  if (lines.empty()) return value;
  return join_lines(lines);
}

inline void set_visible(bool visible) {
  if (title_hidden) {
    title_hidden->set_visible(visible);
  }
  if (hint_hidden) {
    hint_hidden->set_visible(visible);
  }
  if (marker_hidden) {
    marker_hidden->set_visible(false);
  }
  for (auto* hidden : trap_marker_hidden) {
    if (hidden) hidden->set_visible(false);
  }
}

inline void update_line(text::TextObject* text_obj, transform::NoRotationTransform* transform,
                        const std::string& value, float font_px, const glm::vec2& center_norm) {
  if (!text_obj || !transform) return;
  const std::string wrapped =
      wrap_text_for_view(value, font_px, view_width() * kMaxTextWidthRatio);
  text_obj->text = wrapped;
  const auto layout = engine::text::layout_text(wrapped, 0.0f, 0.0f, font_px);
  const glm::vec2 size{layout.width, layout.height};
  const glm::vec2 center = shrooms::screen::norm_to_pixels(center_norm);
  transform->pos = shrooms::screen::center_to_top_left(center, size);
}

inline void set_marker_center(const glm::vec2& center_px) {
  if (!marker_transform) return;
  const glm::vec2 size{marker_radius * 2.0f, marker_radius * 2.0f};
  marker_transform->pos = shrooms::screen::center_to_top_left(center_px, size);
}

inline void show_marker(const glm::vec2& center_px, const engine::UIColor& color) {
  if (!marker_hidden || !marker_circle) return;
  marker_hidden->show();
  marker_circle->color = color;
  set_marker_center(center_px);
}

inline void hide_marker() {
  if (!marker_hidden) return;
  marker_hidden->hide();
}

inline void pause_main_scene(bool paused) {
  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->set_pause(paused);
  }
}

inline void set_trap_marker_center(int index, const glm::vec2& center_px) {
  if (index < 0 || index >= kTrapTargetCount) return;
  auto* transform = trap_marker_transforms[static_cast<size_t>(index)];
  if (!transform) return;
  const glm::vec2 size{marker_radius * 2.0f, marker_radius * 2.0f};
  transform->pos = shrooms::screen::center_to_top_left(center_px, size);
}

inline void hide_trap_markers() {
  for (auto* hidden : trap_marker_hidden) {
    if (hidden) hidden->hide();
  }
}

inline void update_trap_marker_colors() {
  for (int i = 0; i < kTrapTargetCount; ++i) {
    auto* circle = trap_marker_circles[static_cast<size_t>(i)];
    if (!circle) continue;
    circle->color = trap_target_filled[static_cast<size_t>(i)]
                        ? engine::UIColor{0.45f, 0.95f, 0.6f, 0.9f}
                        : engine::UIColor{0.55f, 0.72f, 1.0f, 0.9f};
  }
}

inline void show_trap_markers() {
  update_trap_marker_colors();
  for (int i = 0; i < kTrapTargetCount; ++i) {
    const size_t index = static_cast<size_t>(i);
    if (trap_marker_hidden[index]) trap_marker_hidden[index]->show();
    set_trap_marker_center(i, trap_target_centers_px[index]);
  }
}

inline void clear_stage_entities(bool delete_entities = true) {
  auto clear_one = [&](ecs::Entity*& entity) {
    if (!entity) return;
    if (delete_entities && !entity->is_pending_deletion()) {
      entity->mark_deleted();
    }
    entity = nullptr;
  };
  clear_one(stage_entity_a);
  clear_one(stage_entity_b);
  clear_one(stage_entity_c);
  stage_a_done = false;
  stage_b_done = false;
  stage_c_done = false;
}

inline void clear_recipe_preview() {
  for (auto*& entity : recipe_preview_entities) {
    if (entity && !entity->is_pending_deletion()) {
      entity->mark_deleted();
    }
    entity = nullptr;
  }
}

inline int planted_familiar_count() {
  int count = 0;
  for (auto* logic : player::familiar_logic) {
    if (!logic) continue;
    if (logic->state == player::FamiliarState::Planted) {
      ++count;
    }
  }
  return count;
}

inline bool any_planted_familiar() { return planted_familiar_count() > 0; }

inline bool any_familiar_busy() {
  for (auto* logic : player::familiar_logic) {
    if (!logic) continue;
    if (logic->state != player::FamiliarState::Ready) {
      return true;
    }
  }
  return false;
}

inline float trap_target_y_px() {
  return player_center_px().y - player::player_size.y * 0.65f;
}

inline void reset_trap_targets() {
  trap_target_centers_px = {
      glm::vec2{view_width() * 0.24f, trap_target_y_px()},
      glm::vec2{view_width() * 0.50f, trap_target_y_px()},
      glm::vec2{view_width() * 0.76f, trap_target_y_px()},
  };
  trap_familiar_centers_px.fill(glm::vec2{0.0f, 0.0f});
  trap_target_filled.fill(false);
  trap_target_filled_count = 0;
  update_trap_marker_colors();
}

inline int target_index_for_center(const glm::vec2& center) {
  const float tolerance = std::max(marker_radius * 1.8f, player::player_size.x * 0.7f);
  int best = -1;
  float best_dist = 0.0f;
  for (int i = 0; i < kTrapTargetCount; ++i) {
    const glm::vec2 delta = center - trap_target_centers_px[static_cast<size_t>(i)];
    const float dist = glm::length(delta);
    if (dist > tolerance) continue;
    if (best < 0 || dist < best_dist) {
      best = i;
      best_dist = dist;
    }
  }
  return best;
}

inline bool validate_trap_placements() {
  trap_target_filled.fill(false);
  trap_familiar_centers_px.fill(glm::vec2{0.0f, 0.0f});
  trap_target_filled_count = 0;

  for (auto* logic : player::familiar_logic) {
    if (!logic) continue;
    if (logic->state != player::FamiliarState::Planted) continue;
    const int target_index = target_index_for_center(logic->current_center());
    if (target_index < 0) {
      return false;
    }
    const size_t index = static_cast<size_t>(target_index);
    if (trap_target_filled[index]) {
      return false;
    }
    trap_target_filled[index] = true;
    trap_familiar_centers_px[index] = logic->current_center();
    ++trap_target_filled_count;
  }

  update_trap_marker_colors();
  return true;
}

inline void set_stage(Stage next, const std::string& feedback = "");

inline std::vector<std::pair<std::string, int>> tutorial_recipe() {
  return {{good_mushroom_type, kRecipeGoodTarget}, {bad_mushroom_type, kRecipeBadTarget}};
}

inline void hide_tutorial_lives() {
  score_hud::set_lives_visible(false);
}

inline void show_tutorial_recipe_board() {
  scoreboard::init_with_targets(tutorial_recipe(), "");
  scoreboard::update_score(good_mushroom_type, recipe_good_catches, kRecipeGoodTarget);
  scoreboard::update_score(bad_mushroom_type, 0, kRecipeBadTarget);
}

inline void hide_tutorial_recipe_board() {
  scoreboard::init_with_targets({}, "");
}

inline void restart_stage(const std::string& reason, Stage restart_stage = Stage::None) {
  if (stage_restart_pending) return;
  pending_restart_origin_stage = stage;
  pending_restart_stage = restart_stage == Stage::None ? stage : restart_stage;
  pending_restart_feedback = reason;
  stage_restart_pending = true;
  deferred::fire_deferred(
      []() {
        const Stage restart = pending_restart_stage;
        const Stage origin = pending_restart_origin_stage;
        const std::string feedback = pending_restart_feedback;
        stage_restart_pending = false;
        pending_restart_stage = Stage::None;
        pending_restart_origin_stage = Stage::None;
        pending_restart_feedback.clear();
        if (!active || stage != origin) return;
        set_stage(restart, feedback);
      },
      0);
}

inline ecs::Entity* spawn_single(const std::string& type, float x_px, float y_norm = 0.15f) {
  return levels::spawn_mushroom_now(
      type, glm::vec2{x_px, view_height() * y_norm});
}

inline void spawn_catch_practice_mushroom(const std::string& feedback = "") {
  clear_stage_entities(false);
  static constexpr std::array<float, kPracticeTarget> kCatchLanes{0.28f, 0.72f, 0.42f};
  const size_t lane = static_cast<size_t>(catch_spawn_index % kPracticeTarget);
  stage_entity_a = spawn_single(good_mushroom_type, view_width() * kCatchLanes[lane], 0.12f);
  ++catch_spawn_index;
  update_line(hint_text, hint_transform,
              "Catch " + std::to_string(kPracticeTarget) + " borovik mushrooms: " +
                  std::to_string(catch_practice_catches) + "/" +
                  std::to_string(kPracticeTarget) + "." +
                  (feedback.empty() ? "" : ("  " + feedback)),
              hint_font_px(), kHintCenterNorm);
}

inline void enter_catch_practice_stage(const std::string& feedback = "") {
  clear_stage_entities();
  catch_practice_catches = 0;
  catch_spawn_index = 0;
  spawn_catch_practice_mushroom(feedback);
}

inline void enter_place_three_traps_stage() {
  clear_stage_entities();
  player::reset_familiars();
  reset_trap_targets();
  show_trap_markers();
}

inline void enter_trap_collect_demo_stage() {
  clear_stage_entities();
  hide_trap_markers();
  trap_demo_catches = 0;
  trap_demo_spawned = false;
  const float border_x =
      player_center_x() < view_width() * 0.5f ? view_width() * 0.93f : view_width() * 0.07f;
  trap_border_marker_px = glm::vec2{border_x, view_height() * 0.82f};
  show_marker(trap_border_marker_px, engine::UIColor{0.45f, 0.95f, 0.6f, 0.9f});
}

inline void spawn_trap_demo_mushrooms() {
  if (trap_demo_spawned) return;
  trap_demo_spawned = true;
  player::set_movement_locked(true);
  hide_marker();
  update_line(hint_text, hint_transform,
              "Movement is locked. Let the three bats collect everything.",
              hint_font_px(), kHintCenterNorm);
  stage_entity_a = levels::spawn_mushroom_now(
      good_mushroom_type, glm::vec2{trap_familiar_centers_px[0].x, view_height() * 0.12f});
  stage_entity_b = levels::spawn_mushroom_now(
      good_mushroom_type, glm::vec2{trap_familiar_centers_px[1].x, view_height() * 0.12f});
  stage_entity_c = levels::spawn_mushroom_now(
      good_mushroom_type, glm::vec2{trap_familiar_centers_px[2].x, view_height() * 0.12f});
}

inline void start_pair_practice_countdown(const std::string& feedback = "") {
  if (pair_practice_countdown_started) return;
  pair_practice_countdown_started = true;
  update_line(hint_text, hint_transform,
              "Get ready." + (feedback.empty() ? "" : ("  " + feedback)),
              hint_font_px(), kHintCenterNorm);
  pause_main_scene(true);
  countdown::start("main", 3, []() {
    pause_main_scene(false);
    if (!active || stage != Stage::TrapPairIntro) return;
    set_stage(Stage::TrapPairPractice);
  });
}

inline void enter_pair_practice_intro_stage(const std::string& feedback = "") {
  clear_stage_entities();
  player::reset_familiars();
  pair_practice_countdown_started = false;
  start_pair_practice_countdown(feedback);
}

inline void spawn_pair_practice_pair(const std::string& feedback = "") {
  clear_stage_entities(false);
  static constexpr std::array<std::pair<float, float>, kPracticeTarget> kPairs{
      std::pair<float, float>{0.18f, 0.82f},
      std::pair<float, float>{0.22f, 0.78f},
      std::pair<float, float>{0.14f, 0.86f},
  };
  const auto pair = kPairs[static_cast<size_t>(pair_spawn_index % kPracticeTarget)];
  pair_current_catches = 0;
  stage_entity_a = spawn_single(good_mushroom_type, view_width() * pair.first, 0.12f);
  stage_entity_b = spawn_single(good_mushroom_type, view_width() * pair.second, 0.12f);
  ++pair_spawn_index;
  update_line(hint_text, hint_transform,
              "Use bats and movement to collect three far-apart pairs: " +
                  std::to_string(pair_practice_pairs_completed) + "/" +
                  std::to_string(kPracticeTarget) + "." +
                  (feedback.empty() ? "" : ("  " + feedback)),
              hint_font_px(), kHintCenterNorm);
}

inline void enter_pair_practice_stage(const std::string& feedback = "") {
  clear_stage_entities();
  player::set_movement_locked(false);
  player::reset_familiars();
  pair_practice_pairs_completed = 0;
  pair_spawn_index = 0;
  spawn_pair_practice_pair(feedback);
}

inline glm::vec2 shoot_spawn_center() {
  static constexpr std::array<float, kPracticeTarget> kShootLanes{0.50f, 0.30f, 0.70f};
  const size_t lane = static_cast<size_t>(shoot_spawn_index % kPracticeTarget);
  return glm::vec2{view_width() * kShootLanes[lane], view_height() * 0.12f};
}

inline ecs::Entity* spawn_shoot_mukhomor_target(const glm::vec2& center) {
  const std::string texture_name = bad_mushroom_type.empty() ? "mukhomor" : bad_mushroom_type;
  const glm::vec2 size = shrooms::texture_sizing::from_reference_width(texture_name, 28.0f);
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = shrooms::screen::center_to_top_left(center, size);
  entity->add(transform);
  entity->add(arena::create<geometry::Quad>(
      "tutorial_shoot_mukhomor",
      std::vector<glm::vec2>{
          glm::vec2{0.0f, 0.0f},
          glm::vec2{size.x, 0.0f},
          glm::vec2{0.0f, size.y},
          glm::vec2{size.x, size.y},
      }));
  entity->add(arena::create<layers::ConstLayer>(9));
  const engine::TextureId tex_id = engine::resources::register_texture(texture_name);
  entity->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
  entity->add(arena::create<collision::ColliderObject>("mushroom_catch_handler"));
  entity->add(arena::create<collision::ColliderObject>("bone_projectile_handler"));
  entity->add(arena::create<scene::SceneObject>("main"));
  return entity;
}

inline void spawn_shoot_practice_target(const std::string& feedback = "") {
  clear_stage_entities(false);
  const glm::vec2 center = shoot_spawn_center();
  stage_entity_a = spawn_shoot_mukhomor_target(center);
  ++shoot_spawn_index;
  update_line(hint_text, hint_transform,
              "Shoot three mukhomors: " + std::to_string(shoot_practice_shots) +
                  "/" + std::to_string(kPracticeTarget) + "." +
                  (feedback.empty() ? "" : ("  " + feedback)),
              hint_font_px(), kHintCenterNorm);
}

inline void enter_shoot_practice_stage(const std::string& feedback = "") {
  clear_stage_entities();
  player::reset_familiars();
  shoot_practice_shots = 0;
  shoot_spawn_index = 0;
  spawn_shoot_practice_target(feedback);
}

inline std::array<glm::vec2, 3> recipe_spawn_centers() {
  return {
      glm::vec2{view_width() * 0.50f, view_height() * 0.12f},
      glm::vec2{view_width() * 0.24f, view_height() * 0.12f},
      glm::vec2{view_width() * 0.78f, view_height() * 0.12f},
  };
}

struct PreviewBlink : public dynamic::DynamicObject {
  PreviewBlink() : dynamic::DynamicObject() {}
  ~PreviewBlink() override { Component::component_count--; }

  void update() override {
    if (!entity || entity->is_pending_deletion()) return;
    elapsed += static_cast<float>(ecs::context().delta_seconds);
    if (auto* tint = entity->get<color::OneColor>()) {
      const float alpha = 0.22f + 0.58f * (0.5f + 0.5f * std::sin(elapsed * 12.0f));
      tint->color.w = alpha;
    }
  }

  float elapsed = 0.0f;
};

inline ecs::Entity* spawn_recipe_preview(const std::string& type, const glm::vec2& center) {
  const glm::vec2 size = shrooms::texture_sizing::from_reference_width(type, 28.0f);
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = shrooms::screen::center_to_top_left(center, size);
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(8));
  const engine::TextureId tex_id = engine::resources::register_texture(type);
  entity->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
  entity->add(arena::create<color::OneColor>(glm::vec4{1.0f, 1.0f, 1.0f, 0.45f}));
  entity->add(arena::create<PreviewBlink>());
  entity->add(arena::create<scene::SceneObject>("main"));
  return entity;
}

inline void spawn_recipe_previews() {
  clear_recipe_preview();
  const auto centers = recipe_spawn_centers();
  recipe_preview_entities[0] = spawn_recipe_preview(bad_mushroom_type, centers[0]);
  recipe_preview_entities[1] = spawn_recipe_preview(good_mushroom_type, centers[1]);
  recipe_preview_entities[2] = spawn_recipe_preview(good_mushroom_type, centers[2]);
}

inline void start_recipe_countdown() {
  if (recipe_intro_countdown_started) return;
  recipe_intro_countdown_started = true;
  clear_recipe_preview();
  update_line(hint_text, hint_transform, "Get ready.", hint_font_px(), kHintCenterNorm);
  pause_main_scene(true);
  countdown::start("main", 3, []() {
    pause_main_scene(false);
    if (!active || stage != Stage::RecipeIntro) return;
    set_stage(Stage::RecipeScenario);
  });
}

inline void enter_recipe_intro_stage(const std::string& feedback = "") {
  clear_stage_entities();
  player::reset_familiars();
  recipe_good_catches = 0;
  recipe_bad_removed = false;
  recipe_bad_shot = false;
  recipe_intro_countdown_started = false;
  show_tutorial_recipe_board();
  scoreboard::set_layout(scoreboard::LayoutState::CenterIntro);
  spawn_recipe_previews();
  update_line(hint_text, hint_transform,
              "Recipe: catch 2 borovik and keep mukhomor at 0." +
                  (feedback.empty() ? "" : ("  " + feedback)),
              hint_font_px(), kHintCenterNorm);
}

inline void enter_recipe_scenario_stage() {
  clear_stage_entities();
  clear_recipe_preview();
  player::reset_familiars();
  show_tutorial_recipe_board();
  scoreboard::set_layout(scoreboard::LayoutState::Corner);
  const auto centers = recipe_spawn_centers();
  stage_entity_a = levels::spawn_mushroom_now(bad_mushroom_type, centers[0]);
  stage_entity_b = levels::spawn_mushroom_now(good_mushroom_type, centers[1]);
  stage_entity_c = levels::spawn_mushroom_now(good_mushroom_type, centers[2]);
}

inline void maybe_complete_recipe_scenario() {
  if (recipe_bad_removed && recipe_good_catches >= kRecipeGoodTarget) {
    set_stage(Stage::Complete);
  }
}

inline void set_stage(Stage next, const std::string& feedback) {
  stage = next;
  stage_timer = 0.0f;
  hide_marker();
  hide_trap_markers();
  clear_recipe_preview();
  player::set_movement_locked(false);
  hide_tutorial_lives();

  const std::string base_feedback = feedback.empty() ? "" : ("  " + feedback);
  switch (stage) {
    case Stage::MoveLeft: {
      clear_stage_entities();
      hide_tutorial_recipe_board();
      update_line(title_text, title_transform, "Tutorial: Movement", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Move left with " + controls::bound_key_label(controls::Action::MoveLeft) +
                      " to the glowing marker." + base_feedback,
                  hint_font_px(), kHintCenterNorm);
      show_marker(glm::vec2{view_width() * 0.20f, view_height() * 0.82f},
                  engine::UIColor{0.45f, 0.95f, 0.6f, 0.9f});
      break;
    }
    case Stage::MoveRight: {
      clear_stage_entities();
      update_line(title_text, title_transform, "Tutorial: Movement", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Move right with " + controls::bound_key_label(controls::Action::MoveRight) +
                      " to the glowing marker." + base_feedback,
                  hint_font_px(), kHintCenterNorm);
      show_marker(glm::vec2{view_width() * 0.80f, view_height() * 0.82f},
                  engine::UIColor{0.45f, 0.95f, 0.6f, 0.9f});
      break;
    }
    case Stage::CatchPractice: {
      update_line(title_text, title_transform, "Tutorial: Catch", title_font_px(),
                  kTitleCenterNorm);
      enter_catch_practice_stage(feedback);
      break;
    }
    case Stage::PlaceThreeTraps: {
      update_line(title_text, title_transform, "Tutorial: Bats", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Send three bats with " + controls::bound_key_label(controls::Action::Trap) +
                      " to the blue targets. Off-target bats reset this step." +
                      base_feedback,
                  hint_font_px(), kHintCenterNorm);
      enter_place_three_traps_stage();
      break;
    }
    case Stage::TrapCollectDemo: {
      update_line(title_text, title_transform, "Tutorial: Bat Demo", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Move to the border marker and wait while the bats collect everything." +
                      base_feedback,
                  hint_font_px(), kHintCenterNorm);
      enter_trap_collect_demo_stage();
      break;
    }
    case Stage::TrapPairIntro: {
      update_line(title_text, title_transform, "Tutorial: Bat Practice", title_font_px(),
                  kTitleCenterNorm);
      enter_pair_practice_intro_stage(feedback);
      break;
    }
    case Stage::TrapPairPractice: {
      update_line(title_text, title_transform, "Tutorial: Bat Practice", title_font_px(),
                  kTitleCenterNorm);
      enter_pair_practice_stage(feedback);
      break;
    }
    case Stage::ShootPractice: {
      update_line(title_text, title_transform, "Tutorial: Shooting", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Use " + controls::bound_key_label(controls::Action::Shoot) +
                      " to shoot the mukhomor." +
                      base_feedback,
                  hint_font_px(), kHintCenterNorm);
      enter_shoot_practice_stage(feedback);
      break;
    }
    case Stage::RecipeIntro: {
      update_line(title_text, title_transform, "Tutorial: Recipe", title_font_px(),
                  kTitleCenterNorm);
      enter_recipe_intro_stage(feedback);
      break;
    }
    case Stage::RecipeScenario: {
      update_line(title_text, title_transform, "Tutorial: Recipe", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Catch the two borovik and shoot the mukhomor." +
                      base_feedback,
                  hint_font_px(), kHintCenterNorm);
      enter_recipe_scenario_stage();
      break;
    }
    case Stage::Complete: {
      clear_stage_entities(false);
      hide_tutorial_lives();
      hide_tutorial_recipe_board();
      clear_recipe_preview();
      update_line(title_text, title_transform, "Tutorial Complete", title_font_px(),
                  kTitleCenterNorm);
      update_line(hint_text, hint_transform,
                  "Returning to the menu with Collector selected.", hint_font_px(),
                  kHintCenterNorm);
      levels::finish_tutorial(true);
      break;
    }
    case Stage::None:
    default:
      clear_stage_entities();
      clear_recipe_preview();
      break;
  }
}

inline void start() {
  active = true;
  good_mushroom_type = "borovik";
  bad_mushroom_type = "mukhomor";
  stage_restart_pending = false;
  pending_restart_stage = Stage::None;
  pending_restart_origin_stage = Stage::None;
  pending_restart_feedback.clear();
  player::set_movement_locked(false);
  hide_tutorial_lives();
  hide_tutorial_recipe_board();
  set_visible(true);
  set_stage(Stage::MoveLeft);
}

inline void stop() {
  active = false;
  stage = Stage::None;
  stage_restart_pending = false;
  pending_restart_stage = Stage::None;
  pending_restart_origin_stage = Stage::None;
  pending_restart_feedback.clear();
  clear_stage_entities();
  clear_recipe_preview();
  hide_trap_markers();
  player::set_movement_locked(false);
  hide_tutorial_lives();
  hide_tutorial_recipe_board();
  set_visible(false);
}

inline bool is_active() { return active; }

inline bool is_stage_entity(ecs::Entity* entity, ecs::Entity* tracked) {
  if (!entity || !tracked) return false;
  return entity == tracked;
}

inline void on_mushroom_spawned(const std::string&, ecs::Entity*) {}

inline void on_mushroom_caught(const std::string&, ecs::Entity* entity, bool from_familiar) {
  if (!active || !entity) return;
  if (stage_restart_pending) return;
  if (stage == Stage::CatchPractice) {
    if (is_stage_entity(entity, stage_entity_a)) {
      stage_entity_a = nullptr;
      catch_practice_catches += 1;
      if (catch_practice_catches >= kPracticeTarget) {
        set_stage(Stage::PlaceThreeTraps);
      } else {
        spawn_catch_practice_mushroom();
      }
    }
    return;
  }
  if (stage == Stage::TrapCollectDemo) {
    const bool tracked = is_stage_entity(entity, stage_entity_a) ||
                         is_stage_entity(entity, stage_entity_b) ||
                         is_stage_entity(entity, stage_entity_c);
    if (!tracked) return;
    if (!from_familiar) {
      restart_stage("Let the bats collect these.");
      return;
    }
    if (is_stage_entity(entity, stage_entity_a) && !stage_a_done) {
      stage_a_done = true;
      ++trap_demo_catches;
    } else if (is_stage_entity(entity, stage_entity_b) && !stage_b_done) {
      stage_b_done = true;
      ++trap_demo_catches;
    } else if (is_stage_entity(entity, stage_entity_c) && !stage_c_done) {
      stage_c_done = true;
      ++trap_demo_catches;
    }
    if (trap_demo_catches >= kTrapTargetCount) {
      stage_entity_a = nullptr;
      stage_entity_b = nullptr;
      stage_entity_c = nullptr;
      set_stage(Stage::TrapPairIntro);
    }
    return;
  }
  if (stage == Stage::TrapPairPractice) {
    if (is_stage_entity(entity, stage_entity_a) && !stage_a_done) {
      stage_a_done = true;
      ++pair_current_catches;
    } else if (is_stage_entity(entity, stage_entity_b) && !stage_b_done) {
      stage_b_done = true;
      ++pair_current_catches;
    }
    if (pair_current_catches >= 2) {
      stage_entity_a = nullptr;
      stage_entity_b = nullptr;
      pair_practice_pairs_completed += 1;
      if (pair_practice_pairs_completed >= kPracticeTarget) {
        set_stage(Stage::ShootPractice);
      } else {
        spawn_pair_practice_pair();
      }
    }
    return;
  }
  if (stage == Stage::ShootPractice) {
    if (is_stage_entity(entity, stage_entity_a)) {
      restart_stage("Shoot the mukhomor instead of catching it.");
    }
    return;
  }
  if (stage == Stage::RecipeScenario) {
    if (is_stage_entity(entity, stage_entity_a)) {
      restart_stage("Mukhomor must be shot, not caught.", Stage::RecipeIntro);
      return;
    }
    if (is_stage_entity(entity, stage_entity_b) && !stage_b_done) {
      stage_b_done = true;
      recipe_good_catches += 1;
    } else if (is_stage_entity(entity, stage_entity_c) && !stage_c_done) {
      stage_c_done = true;
      recipe_good_catches += 1;
    }
    scoreboard::update_score(good_mushroom_type, recipe_good_catches, kRecipeGoodTarget);
    maybe_complete_recipe_scenario();
  }
}

inline void on_mushroom_missed(const std::string&, ecs::Entity* entity) {
  if (!active || !entity) return;
  if (stage_restart_pending) return;
  if (stage == Stage::CatchPractice && is_stage_entity(entity, stage_entity_a)) {
    stage_entity_a = nullptr;
    spawn_catch_practice_mushroom("It fell. Try another.");
    return;
  }
  if (stage == Stage::TrapCollectDemo &&
      (is_stage_entity(entity, stage_entity_a) || is_stage_entity(entity, stage_entity_b) ||
       is_stage_entity(entity, stage_entity_c))) {
    restart_stage("A bat missed one. Send the bats again.", Stage::PlaceThreeTraps);
    return;
  }
  if (stage == Stage::TrapPairPractice &&
      (is_stage_entity(entity, stage_entity_a) || is_stage_entity(entity, stage_entity_b))) {
    stage_entity_a = nullptr;
    stage_entity_b = nullptr;
    spawn_pair_practice_pair("Try that pair again.");
    return;
  }
  if (stage == Stage::ShootPractice && is_stage_entity(entity, stage_entity_a)) {
    stage_entity_a = nullptr;
    spawn_shoot_practice_target("Shoot the next mukhomor.");
    return;
  }
  if (stage == Stage::RecipeScenario &&
      (is_stage_entity(entity, stage_entity_a) || is_stage_entity(entity, stage_entity_b) ||
       is_stage_entity(entity, stage_entity_c))) {
    restart_stage("Recipe failed. Watch the preview and try again.", Stage::RecipeIntro);
  }
}

inline void on_mushroom_sorted(const std::string&, ecs::Entity* entity) {
  if (!active || !entity) return;
  if (stage_restart_pending) return;
  if (stage == Stage::CatchPractice && is_stage_entity(entity, stage_entity_a)) {
    restart_stage("This stage needs a catch, not a shot.");
    return;
  }
  if (stage == Stage::TrapCollectDemo &&
      (is_stage_entity(entity, stage_entity_a) || is_stage_entity(entity, stage_entity_b) ||
       is_stage_entity(entity, stage_entity_c))) {
    restart_stage("Let the bats collect these.", Stage::PlaceThreeTraps);
    return;
  }
  if (stage == Stage::TrapPairPractice &&
      (is_stage_entity(entity, stage_entity_a) || is_stage_entity(entity, stage_entity_b))) {
    restart_stage("Borovik belongs in the basket.");
    return;
  }
  if (stage == Stage::ShootPractice && is_stage_entity(entity, stage_entity_a)) {
    stage_entity_a = nullptr;
    shoot_practice_shots += 1;
    if (shoot_practice_shots >= kPracticeTarget) {
      set_stage(Stage::RecipeIntro);
    } else {
      spawn_shoot_practice_target();
    }
    return;
  }
  if (stage == Stage::RecipeScenario) {
    if (is_stage_entity(entity, stage_entity_a)) {
      recipe_bad_removed = true;
      recipe_bad_shot = true;
      stage_a_done = true;
      maybe_complete_recipe_scenario();
      return;
    }
    if (is_stage_entity(entity, stage_entity_b) || is_stage_entity(entity, stage_entity_c)) {
      restart_stage("Borovik belongs in the basket.", Stage::RecipeIntro);
    }
  }
}

struct TutorialController : public dynamic::DynamicObject {
  TutorialController() : dynamic::DynamicObject() {}
  ~TutorialController() override { Component::component_count--; }

  void update() override {
    if (!active) return;
    if (!levels::is_tutorial_mode()) {
      stop();
      return;
    }
    if (scene::is_current_scene_paused()) return;
    stage_timer += static_cast<float>(ecs::context().delta_seconds);

    if (stage == Stage::MoveLeft) {
      if (player_center_x() <= view_width() * 0.22f) {
        set_stage(Stage::MoveRight);
      }
      return;
    }
    if (stage == Stage::MoveRight) {
      if (player_center_x() >= view_width() * 0.78f) {
        set_stage(Stage::CatchPractice);
      }
      return;
    }
    if (stage == Stage::PlaceThreeTraps) {
      const int planted = planted_familiar_count();
      if (planted > 0) {
        if (!validate_trap_placements()) {
          player::reset_familiars();
          reset_trap_targets();
          show_trap_markers();
          update_line(hint_text, hint_transform,
                      "That bat was not on a target. Send all three again.",
                      hint_font_px(), kHintCenterNorm);
          return;
        }
        if (trap_target_filled_count >= kTrapTargetCount) {
          set_stage(Stage::TrapCollectDemo);
          return;
        }
        update_line(hint_text, hint_transform,
                    "Send three bats to the blue targets: " +
                        std::to_string(trap_target_filled_count) + "/" +
                        std::to_string(kTrapTargetCount) + ".",
                    hint_font_px(), kHintCenterNorm);
        return;
      }
      return;
    }
    if (stage == Stage::TrapCollectDemo) {
      if (planted_familiar_count() < kTrapTargetCount && !trap_demo_spawned) {
        set_stage(Stage::PlaceThreeTraps, "Send all three bats first.");
        return;
      }
      const bool reached_marker =
          trap_border_marker_px.x < view_width() * 0.5f
              ? player_center_x() <= view_width() * 0.11f
              : player_center_x() >= view_width() * 0.89f;
      if (reached_marker && !trap_demo_spawned) {
        spawn_trap_demo_mushrooms();
      }
      return;
    }
    if (stage == Stage::RecipeIntro) {
      if (stage_timer >= kRecipePreviewSeconds) {
        start_recipe_countdown();
      }
      return;
    }
  }
};

inline TutorialController tutorial_controller{};

inline void init() {
  levels::set_tutorial_hooks(
      [](const std::string& type, ecs::Entity* entity) { on_mushroom_spawned(type, entity); },
      [](const std::string& type, ecs::Entity* entity, bool from_familiar) {
        on_mushroom_caught(type, entity, from_familiar);
      },
      [](const std::string& type, ecs::Entity* entity) { on_mushroom_missed(type, entity); },
      [](const std::string& type, ecs::Entity* entity) { on_mushroom_sorted(type, entity); });

  title_entity = arena::create<ecs::Entity>();
  title_transform = arena::create<transform::NoRotationTransform>();
  title_entity->add(title_transform);
  title_entity->add(arena::create<layers::ConstLayer>(9));
  title_text = arena::create<text::TextObject>("", 24.0f);
  title_entity->add(title_text);
  title_hidden = arena::create<hidden::HiddenObject>();
  title_entity->add(title_hidden);
  title_entity->add(arena::create<scene::SceneObject>("main"));

  hint_entity = arena::create<ecs::Entity>();
  hint_transform = arena::create<transform::NoRotationTransform>();
  hint_entity->add(hint_transform);
  hint_entity->add(arena::create<layers::ConstLayer>(9));
  hint_text = arena::create<text::TextObject>("", 18.0f);
  hint_entity->add(hint_text);
  hint_color = arena::create<color::OneColor>(glm::vec4{0.95f, 0.95f, 0.95f, 1.0f});
  hint_entity->add(hint_color);
  hint_hidden = arena::create<hidden::HiddenObject>();
  hint_entity->add(hint_hidden);
  hint_entity->add(arena::create<scene::SceneObject>("main"));

  marker_entity = arena::create<ecs::Entity>();
  marker_transform = arena::create<transform::NoRotationTransform>();
  marker_entity->add(marker_transform);
  marker_entity->add(arena::create<layers::ConstLayer>(9));
  marker_radius = std::max(14.0f, shrooms::screen::scale_to_pixels(glm::vec2{0.015f, 0.0f}).x);
  marker_circle = arena::create<render_system::CircleRenderable>(
      marker_radius, engine::UIColor{0.45f, 0.95f, 0.6f, 0.9f});
  marker_entity->add(marker_circle);
  marker_hidden = arena::create<hidden::HiddenObject>();
  marker_entity->add(marker_hidden);
  marker_entity->add(arena::create<scene::SceneObject>("main"));

  for (int i = 0; i < kTrapTargetCount; ++i) {
    const size_t index = static_cast<size_t>(i);
    auto* entity = arena::create<ecs::Entity>();
    auto* transform = arena::create<transform::NoRotationTransform>();
    entity->add(transform);
    entity->add(arena::create<layers::ConstLayer>(9));
    auto* circle = arena::create<render_system::CircleRenderable>(
        marker_radius, engine::UIColor{0.55f, 0.72f, 1.0f, 0.9f});
    entity->add(circle);
    auto* hidden = arena::create<hidden::HiddenObject>();
    entity->add(hidden);
    entity->add(arena::create<scene::SceneObject>("main"));
    trap_marker_entities[index] = entity;
    trap_marker_transforms[index] = transform;
    trap_marker_circles[index] = circle;
    trap_marker_hidden[index] = hidden;
  }

  stop();
}

}  // namespace tutorial
