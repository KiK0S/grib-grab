#pragma once

#include <algorithm>
#include <functional>
#include <string>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "ecs/context.hpp"
#include "utils/arena.hpp"
#include "systems/color/color_system.hpp"
#include "systems/dynamic/dynamic_object.hpp"
#include "systems/hidden/hidden_object.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/scene/scene_system.hpp"
#include "systems/text/text_object.hpp"
#include "systems/transformation/transform_object.hpp"

#include "countdown.hpp"
#include "score_hud.hpp"
#include "scoreboard.hpp"
#include "pixel_snap.hpp"
#include "shrooms_screen.hpp"

namespace level_intro {

struct Config {
  float title_font_px = 30.0f;
  float mode_font_px = 20.0f;
  glm::vec2 title_position_norm{0.0f, 0.48f};
  glm::vec2 mode_position_norm{0.0f, 0.32f};
  glm::vec4 title_color{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 mode_color{0.92f, 0.92f, 0.96f, 1.0f};
  int text_layer = 119;
  float intro_recipe_hold_duration = 1.0f;
  float intro_recipe_move_duration = 2.35f;
  float round_old_shake = 0.45f;
  float round_new_shake = 0.3f;
} config;

enum class IntroPhase {
  Idle,
  RecipeHold,
  RecipeMove,
};

enum class RoundPhase {
  Idle,
  ShakeOld,
  ShakeNew,
};

inline ecs::Entity* title_entity = nullptr;
inline text::TextObject* title_text = nullptr;
inline transform::NoRotationTransform* title_transform = nullptr;
inline color::OneColor* title_color = nullptr;
inline hidden::HiddenObject* title_hidden = nullptr;

inline ecs::Entity* mode_entity = nullptr;
inline text::TextObject* mode_text = nullptr;
inline transform::NoRotationTransform* mode_transform = nullptr;
inline color::OneColor* mode_color = nullptr;
inline hidden::HiddenObject* mode_hidden = nullptr;

inline bool intro_active = false;
inline IntroPhase intro_phase = IntroPhase::Idle;
inline float intro_phase_elapsed = 0.0f;
inline std::function<void()> intro_done = nullptr;
inline bool round_transition_active = false;
inline RoundPhase round_phase = RoundPhase::Idle;
inline float round_phase_elapsed = 0.0f;
inline std::function<void()> round_reconfigure = nullptr;
inline std::function<void()> round_done = nullptr;

inline float phase_duration(RoundPhase phase) {
  switch (phase) {
    case RoundPhase::ShakeOld:
      return config.round_old_shake;
    case RoundPhase::ShakeNew:
      return config.round_new_shake;
    case RoundPhase::Idle:
    default:
      return 0.0f;
  }
}

inline void update_text_layout(text::TextObject* text_obj,
                               transform::NoRotationTransform* transform,
                               const std::string& value, float font_px,
                               const glm::vec2& center_norm) {
  if (!text_obj || !transform) return;
  text_obj->text = value;
  const auto layout = engine::text::layout_text(value, 0.0f, 0.0f, font_px);
  const glm::vec2 size{layout.width, layout.height};
  const glm::vec2 center = shrooms::screen::norm_to_pixels(center_norm);
  transform->pos = shrooms::pixel_snap::centered_top_left(center, size);
}

inline void set_text_visible(bool visible) {
  if (title_hidden) title_hidden->set_visible(visible);
  if (mode_hidden) mode_hidden->set_visible(visible);
}

inline void set_text(const std::string& title, const std::string& mode) {
  update_text_layout(title_text, title_transform, title, config.title_font_px,
                     config.title_position_norm);
  update_text_layout(mode_text, mode_transform, mode, config.mode_font_px,
                     config.mode_position_norm);
}

inline bool is_active() {
  return intro_active || round_transition_active || countdown::is_active();
}

inline void finish_intro(std::function<void()> done) {
  intro_active = false;
  intro_phase = IntroPhase::Idle;
  intro_phase_elapsed = 0.0f;
  intro_done = nullptr;
  set_text_visible(false);
  if (!scoreboard::current_recipe.empty()) {
    scoreboard::animate_to_layout(scoreboard::LayoutState::Corner, 0.12f);
  }
  if (done) {
    done();
  }
}

inline void start_countdown(std::function<void()> done) {
  countdown::start("main", 3, [done = std::move(done)]() mutable {
    finish_intro(std::move(done));
  });
}

inline void start(const std::string& title, const std::string& mode,
                  std::function<void()> done = nullptr) {
  countdown::cancel();
  intro_active = true;
  intro_phase = IntroPhase::Idle;
  intro_phase_elapsed = 0.0f;
  intro_done = nullptr;
  set_text(title, mode);
  set_text_visible(true);
  score_hud::start_intro_slide_in();

  if (!scoreboard::current_recipe.empty()) {
    scoreboard::start_intro_move_to_corner(config.intro_recipe_move_duration);
  }

  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->activate();
    main_scene->set_pause(true);
  }

  start_countdown(std::move(done));
}

inline void start_recipe_then_countdown(const std::string& title, const std::string& mode,
                                        std::function<void()> done = nullptr) {
  if (scoreboard::current_recipe.empty()) {
    start(title, mode, std::move(done));
    return;
  }

  countdown::cancel();
  intro_active = true;
  intro_phase = IntroPhase::RecipeHold;
  intro_phase_elapsed = 0.0f;
  intro_done = std::move(done);
  set_text(title, mode);
  set_text_visible(true);
  score_hud::start_intro_slide_in();
  scoreboard::set_layout(scoreboard::LayoutState::CenterIntro);

  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->activate();
    main_scene->set_pause(true);
  }
}

inline void begin_round_phase(RoundPhase phase) {
  round_phase = phase;
  round_phase_elapsed = 0.0f;
  switch (phase) {
    case RoundPhase::ShakeOld:
      scoreboard::start_shake(config.round_old_shake);
      break;
    case RoundPhase::ShakeNew:
      scoreboard::start_shake(config.round_new_shake,
                              scoreboard::config.shake_amplitude_px * 0.75f);
      break;
    case RoundPhase::Idle:
    default:
      break;
  }
}

inline void finish_round_transition() {
  round_transition_active = false;
  round_phase = RoundPhase::Idle;
  round_phase_elapsed = 0.0f;
  set_text_visible(false);
  scoreboard::set_layout(scoreboard::LayoutState::Corner);

  auto done = std::move(round_done);
  round_done = nullptr;
  round_reconfigure = nullptr;
  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->set_pause(false);
  }
  if (done) done();
}

inline void start_recipe_round_transition(const std::string& title,
                                          const std::string& mode,
                                          std::function<void()> reconfigure,
                                          std::function<void()> done = nullptr) {
  if (round_transition_active) return;
  countdown::cancel();
  intro_active = false;
  round_transition_active = true;
  round_reconfigure = std::move(reconfigure);
  round_done = std::move(done);
  set_text(title, mode);
  set_text_visible(true);

  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->activate();
    main_scene->set_pause(true);
  }

  begin_round_phase(RoundPhase::ShakeOld);
}

struct LevelIntroController : public dynamic::DynamicObject {
  LevelIntroController() : dynamic::DynamicObject() {}
  ~LevelIntroController() override { Component::component_count--; }

  void update() override {
    if (intro_active && intro_phase != IntroPhase::Idle) {
      const float dt = static_cast<float>(ecs::context().delta_seconds);
      intro_phase_elapsed += dt;
      if (intro_phase == IntroPhase::RecipeHold) {
        if (intro_phase_elapsed >= config.intro_recipe_hold_duration) {
          intro_phase = IntroPhase::RecipeMove;
          intro_phase_elapsed = 0.0f;
          scoreboard::animate_to_layout(scoreboard::LayoutState::Corner,
                                        config.intro_recipe_move_duration);
        }
      } else if (intro_phase == IntroPhase::RecipeMove) {
        if (intro_phase_elapsed >= config.intro_recipe_move_duration) {
          auto done = std::move(intro_done);
          intro_done = nullptr;
          intro_phase = IntroPhase::Idle;
          intro_phase_elapsed = 0.0f;
          start_countdown(std::move(done));
        }
      }
    }

    if (!round_transition_active || round_phase == RoundPhase::Idle) return;

    const float dt = static_cast<float>(ecs::context().delta_seconds);
    round_phase_elapsed += dt;
    const float duration = phase_duration(round_phase);
    if (duration > 0.0f && round_phase_elapsed < duration) {
      return;
    }

    switch (round_phase) {
      case RoundPhase::ShakeOld: {
        auto reconfigure = std::move(round_reconfigure);
        round_reconfigure = nullptr;
        if (reconfigure) {
          reconfigure();
        }
        begin_round_phase(RoundPhase::ShakeNew);
        break;
      }
      case RoundPhase::ShakeNew:
        finish_round_transition();
        break;
      case RoundPhase::Idle:
      default:
        break;
    }
  }
};

inline LevelIntroController controller{};

inline ecs::Entity* make_text_entity(text::TextObject*& text_obj,
                                     transform::NoRotationTransform*& transform,
                                     color::OneColor*& color_obj,
                                     hidden::HiddenObject*& hidden_obj,
                                     const glm::vec4& color_value,
                                     float font_px) {
  auto* entity = arena::create<ecs::Entity>();
  transform = arena::create<transform::NoRotationTransform>();
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(config.text_layer));
  text_obj = arena::create<text::TextObject>("", font_px);
  entity->add(text_obj);
  color_obj = arena::create<color::OneColor>(color_value);
  entity->add(color_obj);
  hidden_obj = arena::create<hidden::HiddenObject>();
  hidden_obj->hide();
  entity->add(hidden_obj);
  entity->add(arena::create<scene::SceneObject>("main"));
  return entity;
}

inline void init() {
  title_entity = make_text_entity(title_text, title_transform, title_color, title_hidden,
                                  config.title_color, config.title_font_px);
  mode_entity = make_text_entity(mode_text, mode_transform, mode_color, mode_hidden,
                                 config.mode_color, config.mode_font_px);
  set_text_visible(false);
}

}  // namespace level_intro
