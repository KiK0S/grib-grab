#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <vector>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "systems/animation/animation_system.hpp"
#include "systems/animation/sprite_animation.hpp"
#include "systems/collision/collider_object.hpp"
#include "systems/color/color_system.hpp"
#include "systems/geometry/shapes/quad.hpp"
#include "systems/hidden/hidden_object.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/render/implicit_skeletoned_sprite.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/transformation/transform_object.hpp"
#include "systems/input/input_system.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/moving/moving_object.hpp"
#include "engine/math.h"
#include "engine/resource_ids.h"
#include "ecs/context.hpp"
#include "systems/scene/scene_system.hpp"

#include "level_manager.hpp"
#include "controls.hpp"
#include "game_audio.hpp"
#include "ambient_layers.hpp"
#include "vfx.hpp"
#include "camera_shake.hpp"
#include "score_hud.hpp"
#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"
#include "touchscreen.hpp"

namespace player {

inline ecs::Entity* player_entity = nullptr;
inline transform::NoRotationTransform* player_transform = nullptr;
inline glm::vec2 player_size{0.0f, 0.0f};
inline animation::SpriteAnimation* player_anim = nullptr;
inline color::OneColor* player_color = nullptr;
inline bool player_movement_locked = false;
inline float blocked_movement_flash_timer = 0.0f;
inline constexpr float kBlockedMovementFlashDuration = 0.22f;

struct PlayerVibe : public dynamic::DynamicObject {
  PlayerVibe() : dynamic::DynamicObject() {}
  ~PlayerVibe() override { Component::component_count--; }

  void kick(float amount) {
    recoil = std::min(1.0f, recoil + amount);
  }

  void update() override {
    if (scene::is_current_scene_paused()) return;
    if (!player_transform) return;
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    const float t = static_cast<float>(ecs::context().time_seconds);
    recoil = std::max(0.0f, recoil - recoil_decay * dt);

    const float bob = std::sin(t * bob_speed + phase) * bob_amp_px;
    const float sway = std::sin(t * bob_speed * 0.6f + phase * 1.3f) * sway_amp_px;
    const float recoil_y = -recoil * bob_amp_px * 2.2f;
    const glm::vec2 offset{sway, bob + recoil_y};
    player_transform->translate(offset - last_offset);
    last_offset = offset;
  }

  float bob_amp_px = 0.0f;
  float sway_amp_px = 0.0f;
  float bob_speed = 3.2f;
  float recoil_decay = 7.5f;
  float recoil = 0.0f;
  float phase = 0.0f;
  glm::vec2 last_offset{0.0f, 0.0f};
};

inline PlayerVibe* player_vibe = nullptr;
inline struct PlayerController* player_controller = nullptr;

struct CarriedMarker : public ecs::Component {
  explicit CarriedMarker(ecs::Entity* carrier) : ecs::Component(), carrier(carrier) {}
  ~CarriedMarker() override { Component::component_count--; }

  ecs::Entity* carrier = nullptr;
};

inline void kick_recoil(float amount) {
  if (!player_vibe) return;
  player_vibe->kick(amount);
}

inline void apply_blocked_movement_flash_color() {
  if (!player_color) return;
  if (blocked_movement_flash_timer <= 0.0f) {
    player_color->color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    return;
  }
  const float t =
      std::clamp(blocked_movement_flash_timer / kBlockedMovementFlashDuration, 0.0f, 1.0f);
  const float pulse = 0.45f + 0.55f * std::sin(t * 3.14159265f);
  player_color->color = glm::vec4{1.0f, 1.0f - 0.82f * pulse, 1.0f - 0.82f * pulse, 1.0f};
}

inline void tick_blocked_movement_flash(float dt) {
  if (blocked_movement_flash_timer > 0.0f) {
    blocked_movement_flash_timer = std::max(0.0f, blocked_movement_flash_timer - dt);
  }
  apply_blocked_movement_flash_color();
}

inline void flash_blocked_movement() {
  blocked_movement_flash_timer = kBlockedMovementFlashDuration;
  apply_blocked_movement_flash_color();
}

inline void set_movement_locked(bool locked) {
  player_movement_locked = locked;
  if (!locked) {
    blocked_movement_flash_timer = 0.0f;
    apply_blocked_movement_flash_color();
  }
}

inline bool movement_locked() { return player_movement_locked; }

enum class FamiliarState {
  Ready,
  EmergingTrap,
  Planted,
  Carry,
  EmergingStrike,
  StrikeAscend,
  Returning,
  Sinking,
  Cooldown,
};

inline void update_bat_hud();

inline engine::ShaderId familiar_sleep_shader_id() {
  static const engine::ShaderId id =
      engine::resources::register_shader("implicit_warp_sprite_gray_2d");
  return id;
}

struct FamiliarSprite : public render_system::ImplicitSkeletonedSprite {
  FamiliarSprite(engine::TextureId texture_id, glm::vec2 size,
                 const engine::UIColor& tint = {})
      : render_system::ImplicitSkeletonedSprite(texture_id, size, tint) {}

  engine::ShaderId shader_id() const override {
    return grayscale ? familiar_sleep_shader_id()
                     : render_system::ImplicitSkeletonedSprite::shader_id();
  }

  void emit(engine::RenderPass& pass) override {
    if (!entity || entity->is_pending_deletion()) return;
    auto* transform = entity->get<transform::TransformObject>();
    if (!transform) return;

    engine::UIColor color = tint;
    if (auto* colored = entity->get<color::ColoredObject>()) {
      const auto c = colored->get_color();
      color = engine::UIColor{c.x, c.y, c.z, c.w};
    }

    if (!uploaded) {
      pass.uploads.push_back(engine::GeometryUpload{geometry_id, geometry});
      uploaded = true;
    }

    const glm::vec2 pos = transform->get_pos();
    const float half_w = size.x * 0.5f;
    const float half_h = size.y * 0.5f;
    const float safe_scale = std::max(0.001f, model_scale);
    const engine::Mat4 view_offset =
        engine::mat4_translate(render_system::view_offset_x, render_system::view_offset_y, 0.0f);
    const engine::Mat4 to_center = engine::mat4_translate(pos.x + half_w, pos.y + half_h, 0.0f);
    const engine::Mat4 rotation = engine::mat4_rotate_z(model_rotation_rad);
    const engine::Mat4 scale = engine::mat4_scale(safe_scale, safe_scale, 1.0f);
    const engine::Mat4 from_center = engine::mat4_translate(-half_w, -half_h, 0.0f);

    engine::DrawItem item{};
    item.geometry_id = geometry_id;
    item.model = engine::mat4_mul(
        view_offset, engine::mat4_mul(to_center, engine::mat4_mul(rotation, engine::mat4_mul(scale, from_center))));
    item.color = color;
    item.texture_id = texture_id;

    generated_points.clear();
    if (point_generator) {
      point_generator(static_cast<float>(ecs::context().time_seconds), generated_points);
    }
    const std::vector<render_system::ImplicitWarpPoint>& active_points =
        generated_points.empty() ? control_points : generated_points;
    render_system::append_implicit_warp_uniforms(item, active_points, warp_power, warp_epsilon,
                                                 warp_rest_weight);
    pass.draw_items.push_back(std::move(item));
  }

  void use_idle_wobble(bool enabled) {
    if (enabled) {
      point_generator = idle_point_generator;
    } else {
      point_generator = {};
    }
  }

  void reset_pose() {
    model_scale = 1.0f;
    model_rotation_rad = 0.0f;
  }

  bool grayscale = false;
  float model_scale = 1.0f;
  float model_rotation_rad = 0.0f;
  render_system::ImplicitWarpPointGenerator idle_point_generator{};
};

struct FamiliarLogic : public dynamic::DynamicObject {
  FamiliarLogic() : dynamic::DynamicObject() {}
  ~FamiliarLogic() override { Component::component_count--; }

  void update() override {
    if (scene::is_current_scene_paused()) return;

    if (!transform) {
      transform = entity ? entity->get<transform::NoRotationTransform>() : nullptr;
      if (!transform) return;
    }

    const float dt = static_cast<float>(ecs::context().delta_seconds);

    if (carried && carried->is_pending_deletion()) {
      clear_carried(false);
      begin_return();
    }

    switch (state) {
      case FamiliarState::Ready: {
        set_center(floor_center_for_x(player_center().x));
        break;
      }
      case FamiliarState::EmergingTrap:
      case FamiliarState::EmergingStrike: {
        update_emerge(dt);
        break;
      }
      case FamiliarState::Planted: {
        set_center(planted_center);
        break;
      }
      case FamiliarState::Carry: {
        move_toward_player(dt);
        break;
      }
      case FamiliarState::StrikeAscend: {
        glm::vec2 center = current_center();
        center.x = strike_lane_x;
        center.y -= strike_up_speed * dt;
        set_center(center);
        if (center.y <= strike_top_y) {
          begin_return(strike_return_delay);
        }
        break;
      }
      case FamiliarState::Returning: {
        update_returning(dt);
        break;
      }
      case FamiliarState::Sinking: {
        update_sinking(dt);
        break;
      }
      case FamiliarState::Cooldown: {
        cooldown_timer = std::max(0.0f, cooldown_timer - dt);
        if (cooldown_timer <= 0.0f) {
          begin_ready();
        }
        break;
      }
    }

    update_carried_position();

    if (state == FamiliarState::Carry || state == FamiliarState::StrikeAscend) {
      trail_timer -= dt;
      if (trail_timer <= 0.0f) {
        vfx::spawn_projectile_trail(current_center(), size.x * 0.25f);
        trail_timer = trail_period;
      }
    }

    update_visual_state();
  }

  bool is_idle() const { return state == FamiliarState::Ready; }

  bool can_capture() const { return state == FamiliarState::Planted && !carried; }

  bool can_strike_hit() const { return state == FamiliarState::StrikeAscend; }

  void launch_strike() {
    if (!is_idle()) return;
    const glm::vec2 target = action_center();
    strike_lane_x = target.x;
    strike_top_y = -size.y * 0.6f;
    begin_emerge(target, FamiliarState::EmergingStrike);
    kick_recoil(0.45f);
  }

  void handle_strike_hit(ecs::Entity* mushroom) {
    if (!mushroom || mushroom->is_pending_deletion()) return;
    if (vfx::is_mushroom_vfx_locked(mushroom)) return;
    if (!can_strike_hit()) return;
    levels::on_mushroom_sorted(mushroom);
    begin_return(strike_return_delay);
  }

  void deploy() {
    if (!is_idle()) return;
    planted_center = action_center();
    begin_emerge(planted_center, FamiliarState::EmergingTrap);
  }

  void handle_capture(ecs::Entity* mushroom) {
    if (!mushroom || mushroom->is_pending_deletion()) return;
    if (vfx::is_mushroom_vfx_locked(mushroom)) return;
    if (!can_capture()) return;
    if (mushroom->get<CarriedMarker>()) return;

    mushroom->add(arena::create<CarriedMarker>(entity));
    carried = mushroom;
    carried_transform = mushroom->get<transform::NoRotationTransform>();
    carried_size = vfx::entity_size(mushroom);
    if (carried_size.x <= 0.0f || carried_size.y <= 0.0f) {
      carried_size = size * 0.8f;
    }

    if (auto* moving = mushroom->get<dynamic::MovingObject>()) {
      moving->translate = glm::vec2{0.0f, 0.0f};
    }

    carry_offset = glm::vec2{0.0f, -player_size.y * 0.15f};
    state = FamiliarState::Carry;
    trail_timer = 0.0f;
    vfx::spawn_projectile_flash(current_center(), size);
  }

  void reset() {
    clear_carried(true);
    state = FamiliarState::Ready;
    transition_elapsed = 0.0f;
    return_hold_timer = 0.0f;
    cooldown_timer = 0.0f;
    trail_timer = 0.0f;
    if (!transform) {
      transform = entity ? entity->get<transform::NoRotationTransform>() : nullptr;
    }
    if (sprite) {
      if (normal_texture_id != engine::kInvalidTextureId) {
        sprite->texture_id = normal_texture_id;
      }
      sprite->reset_pose();
      sprite->grayscale = false;
    }
    if (hidden) hidden->hide();
    set_center(floor_center_for_x(player_center().x));
    update_bat_hud();
  }

  void clear_carried(bool delete_entity) {
    if (!carried) return;
    if (delete_entity) {
      carried->mark_deleted();
    }
    carried = nullptr;
    carried_transform = nullptr;
    carried_size = glm::vec2{0.0f, 0.0f};
  }

  void deliver() {
    if (!carried || carried->is_pending_deletion()) {
      clear_carried(false);
      begin_return();
      return;
    }

    const float nan = std::numeric_limits<float>::quiet_NaN();
    glm::vec2 catch_center{nan, nan};
    const glm::vec2 center = player_center();
    if (center.x == center.x && center.y == center.y) {
      catch_center = center;
      if (carried_transform) {
        carried_transform->pos = shrooms::screen::center_to_top_left(center, carried_size);
      }
    }

    auto* sprite = carried->get<render_system::SpriteRenderable>();
    const std::string type = sprite ? engine::resources::texture_name(sprite->texture_id) : "";
    levels::on_mushroom_caught(type, carried, catch_center, true);
    clear_carried(false);
    begin_return();
  }

  void begin_return(float delay = 0.0f) {
    if (sprite) {
      if (normal_texture_id != engine::kInvalidTextureId) {
        sprite->texture_id = normal_texture_id;
      }
      sprite->reset_pose();
      sprite->grayscale = false;
    }
    if (hidden) hidden->show();
    state = FamiliarState::Returning;
    return_hold_timer = std::max(0.0f, delay);
    update_visual_state();
  }

  glm::vec2 current_center() const {
    if (!transform) return glm::vec2{0.0f, 0.0f};
    return transform->pos + size * 0.5f;
  }

  void set_center(const glm::vec2& center) {
    if (!transform) return;
    transform->pos = center - size * 0.5f;
  }

  void update_carried_position() {
    if (!carried || !carried_transform) return;
    const glm::vec2 target_center =
        current_center() + glm::vec2{0.0f, size.y * 0.35f};
    carried_transform->pos = target_center - carried_size * 0.5f;
  }

  void begin_ready() {
    state = FamiliarState::Ready;
    cooldown_timer = 0.0f;
    transition_elapsed = 0.0f;
    if (sprite) {
      if (normal_texture_id != engine::kInvalidTextureId) {
        sprite->texture_id = normal_texture_id;
      }
      sprite->reset_pose();
      sprite->grayscale = false;
    }
    if (hidden) hidden->hide();
    set_center(floor_center_for_x(player_center().x));
    update_bat_hud();
  }

  void begin_emerge(const glm::vec2& target, FamiliarState emerge_state) {
    state = emerge_state;
    transition_elapsed = 0.0f;
    emerge_target = target;
    emerge_start = floor_center_for_x(target.x);
    set_center(emerge_start);
    if (hidden) hidden->show();
    if (sprite) {
      if (normal_texture_id != engine::kInvalidTextureId) {
        sprite->texture_id = normal_texture_id;
      }
      sprite->model_scale = 0.25f;
      sprite->model_rotation_rad = 0.0f;
      sprite->grayscale = false;
    }
    trail_timer = 0.0f;
    update_bat_hud();
  }

  void update_emerge(float dt) {
    transition_elapsed += dt;
    const float t = smooth01(emerge_duration > 0.0f ? transition_elapsed / emerge_duration : 1.0f);
    set_center(lerp(emerge_start, emerge_target, t));
    if (sprite) {
      sprite->model_scale = 0.25f + 0.75f * t;
      sprite->model_rotation_rad = 0.0f;
    }
    if (t < 1.0f) return;

    set_center(emerge_target);
    if (state == FamiliarState::EmergingTrap) {
      state = FamiliarState::Planted;
    } else {
      begin_strike_dash();
    }
  }

  void move_toward_player(float dt) {
    glm::vec2 center = current_center();
    const glm::vec2 target = player_center() + carry_offset;
    const glm::vec2 to_player = target - center;
    const float dist = glm::length(to_player);
    if (dist <= carry_speed * dt + 1.0f) {
      set_center(target);
      deliver();
    } else if (dist > 0.0001f) {
      center += glm::normalize(to_player) * carry_speed * dt;
      set_center(center);
    }
  }

  void update_returning(float dt) {
    if (return_hold_timer > 0.0f) {
      return_hold_timer = std::max(0.0f, return_hold_timer - dt);
      return;
    }

    glm::vec2 center = current_center();
    const glm::vec2 target = action_center();
    const glm::vec2 to_player = target - center;
    const float dist = glm::length(to_player);
    if (dist > return_speed * dt + 1.0f && dist > 0.0001f) {
      center += glm::normalize(to_player) * return_speed * dt;
      set_center(center);
      return;
    }

    set_center(target);
    begin_sink();
  }

  void begin_sink() {
    shrooms::audio::play_familiar_return();
    state = FamiliarState::Sinking;
    transition_elapsed = 0.0f;
    sink_start = current_center();
    sink_target = floor_center_for_x(player_center().x);
    if (sprite) {
      if (normal_texture_id != engine::kInvalidTextureId) {
        sprite->texture_id = normal_texture_id;
      }
      sprite->reset_pose();
      sprite->grayscale = false;
    }
  }

  void update_sinking(float dt) {
    transition_elapsed += dt;
    const float t = smooth01(sink_duration > 0.0f ? transition_elapsed / sink_duration : 1.0f);
    set_center(lerp(sink_start, sink_target, t));
    if (sprite) {
      sprite->model_scale = std::max(0.05f, 1.0f - 0.75f * t);
      sprite->model_rotation_rad = 0.0f;
    }
    if (t < 1.0f) return;

    if (hidden) hidden->hide();
    cooldown_timer = return_delay;
    state = FamiliarState::Cooldown;
  }

  void begin_strike_dash() {
    state = FamiliarState::StrikeAscend;
    shrooms::audio::play_familiar_shot();
    if (sprite) {
      if (strike_texture_id != engine::kInvalidTextureId) {
        sprite->texture_id = strike_texture_id;
      }
      sprite->reset_pose();
      sprite->grayscale = false;
    }
    trail_timer = 0.0f;
    vfx::spawn_projectile_flash(current_center(), size);
  }

  glm::vec2 player_center() const {
    if (!player_transform) {
      return current_center();
    }
    return player_transform->pos + player_size * 0.5f;
  }

  glm::vec2 action_center() const {
    return player_center() + glm::vec2{0.0f, -player_size.y * 0.65f};
  }

  glm::vec2 floor_center_for_x(float x) const {
    glm::vec2 floor_top_left{0.0f, 0.0f};
    glm::vec2 floor_size{0.0f, 0.0f};
    if (ambient_layers::resolve_bottom_sprite_bounds(floor_top_left, floor_size)) {
      const float min_x = floor_top_left.x + size.x * 0.5f;
      const float max_x = floor_top_left.x + floor_size.x - size.x * 0.5f;
      const float clamped_x = min_x <= max_x ? std::clamp(x, min_x, max_x) : x;
      return glm::vec2{clamped_x, floor_top_left.y + floor_size.y * 0.52f};
    }
    return action_center();
  }

  float smooth01(float value) const {
    const float t = std::clamp(value, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
  }

  glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, float t) const {
    return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
  }

  void update_visual_state() {
    const bool visible = state != FamiliarState::Ready && state != FamiliarState::Cooldown;
    if (hidden) hidden->set_visible(visible);
    if (!sprite) return;
    const bool transition =
        state == FamiliarState::EmergingTrap || state == FamiliarState::EmergingStrike ||
        state == FamiliarState::Sinking;
    sprite->use_idle_wobble(state != FamiliarState::StrikeAscend);
    if (!transition) {
      sprite->reset_pose();
    }
    sprite->grayscale = false;
  }

  transform::NoRotationTransform* transform = nullptr;
  FamiliarSprite* sprite = nullptr;
  hidden::HiddenObject* hidden = nullptr;
  glm::vec2 size{0.0f, 0.0f};
  engine::TextureId normal_texture_id = engine::kInvalidTextureId;
  engine::TextureId strike_texture_id = engine::kInvalidTextureId;
  float carry_speed = 820.0f;
  float strike_up_speed = 980.0f;
  float strike_top_y = -50.0f;
  float strike_lane_x = 0.0f;
  float strike_return_delay = 0.22f;
  float return_speed = 720.0f;
  float return_delay = 1.0f;
  float return_hold_timer = 0.0f;
  float transition_elapsed = 0.0f;
  float emerge_duration = 0.22f;
  float sink_duration = 0.24f;
  float cooldown_timer = 0.0f;
  float trail_timer = 0.0f;
  float trail_period = 0.05f;
  glm::vec2 carry_offset{0.0f, 0.0f};
  glm::vec2 planted_center{0.0f, 0.0f};
  glm::vec2 emerge_start{0.0f, 0.0f};
  glm::vec2 emerge_target{0.0f, 0.0f};
  glm::vec2 sink_start{0.0f, 0.0f};
  glm::vec2 sink_target{0.0f, 0.0f};
  FamiliarState state = FamiliarState::Ready;
  ecs::Entity* carried = nullptr;
  transform::NoRotationTransform* carried_transform = nullptr;
  glm::vec2 carried_size{0.0f, 0.0f};
};

inline constexpr int kFamiliarCount = 3;
inline std::array<ecs::Entity*, kFamiliarCount> familiar_entities{};
inline std::array<FamiliarLogic*, kFamiliarCount> familiar_logic{};
inline glm::vec2 familiar_size{0.0f, 0.0f};

inline int ready_familiar_count() {
  int count = 0;
  for (auto* logic : familiar_logic) {
    if (!logic) continue;
    if (logic->is_idle()) ++count;
  }
  return count;
}

inline void update_bat_hud() {
  score_hud::set_bat_availability(ready_familiar_count());
}

inline FamiliarLogic* find_idle_familiar() {
  for (auto* logic : familiar_logic) {
    if (!logic) continue;
    if (logic->is_idle()) return logic;
  }
  return nullptr;
}

inline FamiliarLogic* find_idle_familiar_for_strike() {
  return find_idle_familiar();
}

inline void deploy_familiar() {
  if (auto* logic = find_idle_familiar()) {
    logic->deploy();
  }
}

inline void launch_strike_familiar() {
  if (auto* logic = find_idle_familiar_for_strike()) {
    logic->launch_strike();
  }
}

inline collision::TriggerObject* make_familiar_trigger(FamiliarLogic* logic) {
  return arena::create<collision::TriggerObject>(
      "mushroom_catch_handler",
      [logic](ecs::Entity*, collision::ColliderObject* collider) {
        if (!logic || !collider) return;
        if (!logic->can_capture()) return;
        auto* entity = collider->get_entity();
        if (!entity || entity->is_pending_deletion()) return;
        if (vfx::is_mushroom_vfx_locked(entity)) return;
        logic->handle_capture(entity);
      });
}

inline collision::TriggerObject* make_familiar_sort_trigger(FamiliarLogic* logic) {
  return arena::create<collision::TriggerObject>(
      "bone_projectile_handler",
      [logic](ecs::Entity*, collision::ColliderObject* collider) {
        if (!logic || !collider) return;
        auto* entity = collider->get_entity();
        if (!entity || entity->is_pending_deletion()) return;
        if (vfx::is_mushroom_vfx_locked(entity)) return;
        logic->handle_strike_hit(entity);
      });
}

inline void init_familiars() {
  if (!player_transform) return;
  familiar_size = shrooms::texture_sizing::from_reference_width("famiriar", 29.0f);
  const engine::TextureId tex_id = engine::resources::register_texture("famiriar");
  const engine::TextureId strike_tex_id = engine::resources::register_texture("familiar_attack");

  for (int i = 0; i < kFamiliarCount; ++i) {
    auto* entity = arena::create<ecs::Entity>();
    auto* transform = arena::create<transform::NoRotationTransform>();
    const glm::vec2 player_center =
        player_transform->pos + glm::vec2{player_size.x * 0.5f, player_size.y * 0.5f};
    glm::vec2 spawn_center = player_center;
    glm::vec2 floor_top_left{0.0f, 0.0f};
    glm::vec2 floor_size{0.0f, 0.0f};
    if (ambient_layers::resolve_bottom_sprite_bounds(floor_top_left, floor_size)) {
      spawn_center = glm::vec2{player_center.x, floor_top_left.y + floor_size.y * 0.52f};
    }
    transform->pos = shrooms::screen::center_to_top_left(spawn_center, familiar_size);
    entity->add(transform);

    entity->add(arena::create<geometry::Quad>(
        "familiar_collider",
        std::vector<glm::vec2>{
            glm::vec2{0.0f, 0.0f},
            glm::vec2{familiar_size.x, 0.0f},
            glm::vec2{0.0f, familiar_size.y},
            glm::vec2{familiar_size.x, familiar_size.y},
        }));

    entity->add(arena::create<layers::ConstLayer>(3));
    auto* familiar_sprite =
        arena::create<FamiliarSprite>(tex_id, familiar_size);
    familiar_sprite->warp_power = 2.0f;
    familiar_sprite->warp_epsilon = 0.02f;
    familiar_sprite->warp_rest_weight = 1.25f;
    familiar_sprite->point_generator =
        [](float time_seconds, std::vector<render_system::ImplicitWarpPoint>& out_points) {
          out_points.clear();
          out_points.reserve(9);

          auto add_point = [&](float from_x, float from_y, float to_x, float to_y,
                               float radius_uv) {
            const float clamped_from_x = std::clamp(from_x, 0.0f, 1.0f);
            const float clamped_from_y = std::clamp(from_y, 0.0f, 1.0f);
            const float clamped_to_x = std::clamp(to_x, 0.0f, 1.0f);
            const float clamped_to_y = std::clamp(to_y, 0.0f, 1.0f);
            out_points.push_back(render_system::ImplicitWarpPoint{
                engine::Vec2{clamped_from_x, clamped_from_y},
                engine::Vec2{clamped_to_x, clamped_to_y},
                std::max(0.0f, radius_uv),
            });
          };

          // Keep sprite corners pinned to avoid border drift.
          add_point(0.0f, 0.0f, 0.0f, 0.0f, 0.65f);
          add_point(1.0f, 0.0f, 1.0f, 0.0f, 0.65f);
          add_point(0.0f, 1.0f, 0.0f, 1.0f, 0.65f);
          add_point(1.0f, 1.0f, 1.0f, 1.0f, 0.65f);

          // Hold near extremes and snap between them for a flicker-like motion.
          auto flicker_wave = [](float t) {
            const float hold = 0.38f;
            const float travel = 0.12f;
            const float cycle = hold + travel + hold + travel;
            float p = std::fmod(t, cycle);
            if (p < 0.0f) p += cycle;

            auto smooth01 = [](float u) {
              u = std::clamp(u, 0.0f, 1.0f);
              return u * u * (3.0f - 2.0f * u);
            };

            if (p < hold) return 1.0f;
            p -= hold;
            if (p < travel) {
              const float u = smooth01(p / travel);
              return 1.0f - 2.0f * u;
            }
            p -= travel;
            if (p < hold) return -1.0f;
            p -= hold;
            const float u = smooth01(p / travel);
            return -1.0f + 2.0f * u;
          };

          const float phase = time_seconds * 4.0f;
          const float wave = flicker_wave(phase);

          // Joint layout authored in texture-space pixels.
          constexpr float kTexWidth = 28.5f;
          constexpr float kTexHeight = 14.25f;
          auto to_uv_x = [](float px) { return px / kTexWidth; };
          auto to_uv_y = [](float py) { return py / kTexHeight; };

          const float left_inner_x_base = to_uv_x(10.0f);
          const float left_inner_y_base = to_uv_y(4.0f);
          const float right_inner_x_base = to_uv_x(21.0f);
          const float right_inner_y_base = to_uv_y(2.0f);
          const float left_outer_x_base = to_uv_x(0.0f);
          const float left_outer_y_base = to_uv_y(11.0f);
          const float right_outer_x_base = to_uv_x(28.5f);
          const float right_outer_y_base = to_uv_y(9.0f);

          // Center joint: midpoint of inner joints.
          const float center_x = 0.5f * (left_inner_x_base + right_inner_x_base);
          const float center_y_base = 0.5f * (left_inner_y_base + right_inner_y_base);
          const float center_y_delta = 0.07f * wave;
          const float center_y = center_y_base + center_y_delta;

          const float inner_x_delta = 0.035f * wave;
          const float left_inner_x = left_inner_x_base + inner_x_delta;
          const float right_inner_x = right_inner_x_base - inner_x_delta;

          // Give outer joints a slightly stronger swing for more expression.
          const float outer_x_delta = 0.052f * wave;
          const float left_outer_x = left_inner_x_base + outer_x_delta;
          const float right_outer_x = right_inner_x_base - outer_x_delta;

          add_point(left_outer_x_base, left_outer_y_base, left_outer_x, center_y, 0.42f);
          add_point(left_inner_x_base, left_inner_y_base, left_inner_x, center_y, 0.34f);
          add_point(center_x, center_y_base, center_x, center_y, 0.30f);
          add_point(right_inner_x_base, right_inner_y_base, right_inner_x, center_y, 0.34f);
          add_point(right_outer_x_base, right_outer_y_base, right_outer_x, center_y, 0.42f);
        };
    familiar_sprite->idle_point_generator = familiar_sprite->point_generator;
    entity->add(familiar_sprite);
    auto* hidden = arena::create<hidden::HiddenObject>();
    hidden->hide();
    entity->add(hidden);
    auto* logic = arena::create<FamiliarLogic>();
    logic->size = familiar_size;
    logic->sprite = familiar_sprite;
    logic->hidden = hidden;
    logic->normal_texture_id = tex_id;
    logic->strike_texture_id = strike_tex_id;
    entity->add(logic);
    entity->add(make_familiar_trigger(logic));
    entity->add(make_familiar_sort_trigger(logic));
    entity->add(arena::create<scene::SceneObject>("main"));

    familiar_entities[i] = entity;
    familiar_logic[i] = logic;
  }
  update_bat_hud();
}

inline void reset_familiars() {
  for (auto* logic : familiar_logic) {
    if (!logic) continue;
    logic->reset();
  }
}

struct PlayerController : public dynamic::DynamicObject {
  explicit PlayerController(float step_px) : dynamic::DynamicObject(), step_px(step_px) {}
  ~PlayerController() override { Component::component_count--; }

  void update() override {
    if (scene::is_current_scene_paused()) return;
    if (!player_transform) return;

    const float dt = static_cast<float>(ecs::context().delta_seconds);
    tick_blocked_movement_flash(dt);

    const bool deploy_pressed =
        controls::is_down(controls::Action::Trap) || touchscreen::deploy_pressed;
    if (deploy_pressed && !deploy_pressed_last) {
      deploy_familiar();
    }
    deploy_pressed_last = deploy_pressed;

    const bool fire_pressed = levels::shooting_enabled() &&
        (controls::is_down(controls::Action::Shoot) || touchscreen::fire_pressed);
    if (fire_pressed && !fire_pressed_last) {
      launch_strike_familiar();
    }
    fire_pressed_last = fire_pressed;

    float dx = 0.0f;
    if (controls::is_down(controls::Action::MoveLeft)) dx -= step_px;
    if (controls::is_down(controls::Action::MoveRight)) dx += step_px;
    dx += touchscreen::joystick_value.x * step_px;

    if (player_movement_locked) {
      if (std::abs(dx) > 0.001f) {
        flash_blocked_movement();
      }
      dust_timer = 0.0f;
      return;
    }

    if (dx == 0.0f) {
      dust_timer = 0.0f;
      return;
    }

    player_transform->pos.x += dx;
    const float min_x = 0.0f;
    const float max_x = static_cast<float>(shrooms::screen::view_width) - player_size.x;
    if (player_transform->pos.x < min_x) player_transform->pos.x = min_x;
    if (player_transform->pos.x > max_x) player_transform->pos.x = max_x;

    spawn_dash_dust(dx);

    if (player_anim) {
      if (dx < 0.0f) {
        player_anim->set_state("left");
      } else if (dx > 0.0f) {
        player_anim->set_state("right");
      }
    }
  }

  void spawn_dash_dust(float dx) {
    const float dt = static_cast<float>(ecs::context().delta_seconds);
    dust_timer -= dt;
    if (dust_timer > 0.0f) return;
    dust_timer = dust_period;

    if (!player_transform) return;
    const float direction = dx < 0.0f ? -1.0f : 1.0f;
    const glm::vec2 foot{
        player_transform->pos.x + player_size.x * (direction < 0.0f ? 0.25f : 0.75f),
        player_transform->pos.y + player_size.y * 0.92f,
    };
    vfx::SporeConfig config{};
    config.color = glm::vec4{0.35f, 0.2f, 0.45f, 0.4f};
    config.lifetime = 0.22f;
    config.start_radius = 2.5f;
    config.end_radius = 6.0f;
    config.velocity = glm::vec2{direction * 18.0f, 20.0f};
    config.layer = 2;
    vfx::spawn_spore(foot, config);
  }

  float step_px = 0.0f;
  float dust_timer = 0.0f;
  float dust_period = 0.08f;
  bool deploy_pressed_last = false;
  bool fire_pressed_last = false;
};

inline collision::TriggerObject* make_player_trigger() {
  return arena::create<collision::TriggerObject>(
      "mushroom_catch_handler",
      [](ecs::Entity*, collision::ColliderObject* collider) {
        if (!collider) return;
        auto* entity = collider->get_entity();
        if (!entity || entity->is_pending_deletion()) return;
        if (vfx::is_mushroom_vfx_locked(entity)) return;
        if (entity->get<CarriedMarker>()) return;
        auto* sprite = entity->get<render_system::SpriteRenderable>();
        const std::string type = sprite ? engine::resources::texture_name(sprite->texture_id) : "";
        const float nan = std::numeric_limits<float>::quiet_NaN();
        glm::vec2 catch_center{nan, nan};
        if (player_transform) {
          catch_center = player_transform->pos + player_size * 0.5f;
        }
        levels::on_mushroom_caught(type, entity, catch_center, false);
      });
}

inline void reset_for_new_level() {
  if (!player_transform) return;
  set_movement_locked(false);
  glm::vec2 center = shrooms::screen::norm_to_pixels(glm::vec2{0.0f, -0.6f});
  player_transform->pos = shrooms::screen::center_to_top_left(center, player_size);
  reset_familiars();
}

inline void init() {
  player_entity = arena::create<ecs::Entity>();
  player_size = shrooms::texture_sizing::from_reference_width("witch_right_1", 34.0f);

  player_transform = arena::create<transform::NoRotationTransform>();
  glm::vec2 center = shrooms::screen::norm_to_pixels(glm::vec2{0.0f, -0.6f});
  player_transform->pos = shrooms::screen::center_to_top_left(center, player_size);
  player_entity->add(player_transform);

  player_entity->add(arena::create<geometry::Quad>(
      "player_collider",
      std::vector<glm::vec2>{
          glm::vec2{0.0f, 0.0f},
          glm::vec2{player_size.x, 0.0f},
          glm::vec2{0.0f, player_size.y},
          glm::vec2{player_size.x, player_size.y},
      }));

  player_entity->add(make_player_trigger());

  const engine::TextureId tex_id =
      engine::resources::register_texture("witch_right_1");
  player_entity->add(arena::create<render_system::SpriteRenderable>(tex_id, player_size));
  player_color = arena::create<color::OneColor>(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
  player_entity->add(player_color);

  std::map<std::string, std::vector<animation::SpriteFrame>> clips{};
  clips["left"] = {animation::SpriteFrame{engine::resources::register_texture("witch_left_1"),
                                          0.15f},
                   animation::SpriteFrame{engine::resources::register_texture("witch_left_2"),
                                          0.15f}};
  clips["right"] = {animation::SpriteFrame{engine::resources::register_texture("witch_right_1"),
                                           0.15f},
                    animation::SpriteFrame{engine::resources::register_texture("witch_right_2"),
                                           0.15f}};

  player_anim = arena::create<animation::SpriteAnimation>(std::move(clips), "right");
  player_entity->add(player_anim);

  const float step_px = shrooms::screen::scale_to_pixels(glm::vec2{0.02f, 0.0f}).x * 0.5f;
  player_controller = arena::create<PlayerController>(step_px);
  player_entity->add(player_controller);
  player_vibe = arena::create<PlayerVibe>();
  player_vibe->bob_amp_px = shrooms::screen::scale_to_pixels(glm::vec2{0.0f, 0.01f}).y;
  player_vibe->sway_amp_px = shrooms::screen::scale_to_pixels(glm::vec2{0.01f, 0.0f}).x;
  player_vibe->phase = 1.3f;
  player_entity->add(player_vibe);
  camera_shake::attach(player_entity, camera_shake::AxisMode::XOnly, 1.8f);
  player_entity->add(arena::create<scene::SceneObject>("main"));

  init_familiars();
}

}  // namespace player
