#pragma once

#include <algorithm>
#include <functional>
#include <string>

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

#include "shrooms_screen.hpp"
#include "pixel_snap.hpp"
#include "vfx.hpp"

namespace round_transition {

struct Config {
  glm::vec4 overlay_color{0.92f, 0.88f, 0.98f, 0.35f};
  float overlay_fade_in = 0.25f;
  float overlay_hold = 0.55f;
  float overlay_fade_out = 0.35f;
  float text_delay = 0.05f;
  float text_fade_in = 0.2f;
  float text_fade_out = 0.35f;
  float text_font_px = 26.0f;
  glm::vec2 text_position_norm{0.0f, 0.0f};
  glm::vec4 text_color{1.0f, 1.0f, 1.0f, 1.0f};
  int overlay_layer = 108;
  int text_layer = 109;
  int ambient_layer = 107;
  int ambient_spores = 10;
  float ambient_lifetime = 1.6f;
  float ambient_min_radius = 6.0f;
  float ambient_max_radius = 14.0f;
  float ambient_speed = 12.0f;
  glm::vec4 ambient_color{0.9f, 0.85f, 1.0f, 0.4f};
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
inline std::function<void()> on_done = nullptr;
inline std::function<void()> on_covered = nullptr;
inline bool covered_fired = false;

inline float clamp01(float v) {
  return std::min(1.0f, std::max(0.0f, v));
}

inline float fade_value(float t, float fade_in, float hold, float fade_out) {
  if (fade_in <= 0.0f && fade_out <= 0.0f) return 1.0f;
  const float total_in = std::max(0.0f, fade_in);
  const float total_hold = std::max(0.0f, hold);
  const float total_out = std::max(0.0f, fade_out);
  if (t < total_in) {
    return total_in > 0.0f ? clamp01(t / total_in) : 1.0f;
  }
  if (t < total_in + total_hold) {
    return 1.0f;
  }
  const float out_t = t - total_in - total_hold;
  if (out_t < total_out) {
    return 1.0f - (total_out > 0.0f ? clamp01(out_t / total_out) : 1.0f);
  }
  return 0.0f;
}

inline float total_duration() {
  return std::max(0.0f, config.overlay_fade_in) +
         std::max(0.0f, config.overlay_hold) +
         std::max(0.0f, config.overlay_fade_out);
}

inline void set_overlay_alpha(float alpha) {
  if (!overlay_quad) return;
  overlay_quad->color = engine::UIColor{config.overlay_color.x, config.overlay_color.y,
                                        config.overlay_color.z, clamp01(alpha)};
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
  text_transform->pos = shrooms::pixel_snap::centered_top_left(center, size);
}

inline void spawn_ambient_spores() {
  if (config.ambient_spores <= 0) return;
  const float width = static_cast<float>(shrooms::screen::view_width);
  const float height = static_cast<float>(shrooms::screen::view_height);
  for (int i = 0; i < config.ambient_spores; ++i) {
    const float x = static_cast<float>(rnd::get_double(0.0, width));
    const float y = static_cast<float>(rnd::get_double(height * 0.2, height * 0.85));
    vfx::SporeConfig spore{};
    spore.color = config.ambient_color;
    spore.lifetime =
        static_cast<float>(rnd::get_double(config.ambient_lifetime * 0.8,
                                           config.ambient_lifetime * 1.2));
    spore.start_radius =
        static_cast<float>(rnd::get_double(config.ambient_min_radius, config.ambient_max_radius));
    spore.end_radius = spore.start_radius * 2.2f;
    spore.velocity = glm::vec2{0.0f, -config.ambient_speed};
    spore.layer = config.ambient_layer;
    vfx::spawn_spore(glm::vec2{x, y}, spore);
  }
}

inline void finish_sequence() {
  if (overlay_hidden) overlay_hidden->hide();
  if (text_hidden) text_hidden->hide();
  active = false;
  elapsed = 0.0f;

  auto done = std::move(on_done);
  on_done = nullptr;
  on_covered = nullptr;
  covered_fired = false;
  if (done) {
    done();
    return;
  }
  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->set_pause(false);
  }
}

inline void start_transition(const std::string& message, std::function<void()> done = nullptr,
                             std::function<void()> covered = nullptr) {
  if (active) return;
  active = true;
  elapsed = 0.0f;
  on_done = std::move(done);
  on_covered = std::move(covered);
  covered_fired = false;

  update_text_layout(message);
  set_overlay_alpha(0.0f);
  set_text_alpha(0.0f);

  if (overlay_hidden) overlay_hidden->show();
  if (text_hidden) text_hidden->show();

  spawn_ambient_spores();

  if (auto* main_scene = scene::get_scene("main")) {
    main_scene->activate();
    main_scene->set_pause(true);
  }
}

inline void start_round_win(int current_round, int next_round,
                            std::function<void()> done = nullptr,
                            std::function<void()> covered = nullptr) {
  const std::string message = "Round " + std::to_string(current_round) +
                              " won!\nAdvancing to round " + std::to_string(next_round);
  start_transition(message, std::move(done), std::move(covered));
}

inline void start_level_completed(std::function<void()> done = nullptr) {
  start_transition("level completed", std::move(done));
}

inline bool is_active() { return active; }

struct RoundTransitionController : public dynamic::DynamicObject {
  RoundTransitionController() : dynamic::DynamicObject() {}
  ~RoundTransitionController() override { Component::component_count--; }

  void update() override {
    if (!active) return;

    const float dt = static_cast<float>(ecs::context().delta_seconds);
    elapsed += dt;

    const float overlay_t =
        fade_value(elapsed, config.overlay_fade_in, config.overlay_hold, config.overlay_fade_out);
    set_overlay_alpha(config.overlay_color.w * overlay_t);

    if (!covered_fired && elapsed >= std::max(0.0f, config.overlay_fade_in)) {
      covered_fired = true;
      auto covered = std::move(on_covered);
      on_covered = nullptr;
      if (covered) {
        covered();
      }
    }

    const float text_elapsed = std::max(0.0f, elapsed - config.text_delay);
    const float text_t =
        fade_value(text_elapsed, config.text_fade_in, config.overlay_hold, config.text_fade_out);
    set_text_alpha(config.text_color.w * text_t);

    if (elapsed >= total_duration()) {
      finish_sequence();
    }
  }
};

inline RoundTransitionController controller{};

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
  text_color = arena::create<color::OneColor>(config.text_color);
  text_color->color.w = 0.0f;
  text_entity->add(text_color);
  text_hidden = arena::create<hidden::HiddenObject>();
  text_hidden->hide();
  text_entity->add(text_hidden);
  text_entity->add(arena::create<scene::SceneObject>("main"));
}

}  // namespace round_transition
