#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

#include "engine/resource_ids.h"
#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "ecs/context.hpp"
#include "utils/arena.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/render/render_system.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/scene/scene_object.hpp"
#include "systems/transformation/transform_object.hpp"

#include "camera_shake.hpp"
#include "game_over_sequence.hpp"
#include "panel_occlusion_fx.hpp"
#include "shrooms_screen.hpp"
#include "player.hpp"
#include "visual_constants.hpp"

namespace global_fx {

struct Config {
  glm::vec4 color_tint{0.0f, 0.0f, 0.0f, 0.20f};
  float glow_radius_scale = 0.09f;
  float glow_edge_scale = 0.06f;
  float glow_edge_power = 1.8f;
  float glow_noise_strength = 0.08f;
  float glow_noise_scale = 0.015f;
  float glow_mask_strength = 0.7f;
  float glow_blur_px = 9.0f;
  float glow_divide_strength = 0.55f;
  float glow_divide_epsilon = 0.2f;
  float shake_pad_px = 18.0f;
  int tint_layer = 30;
  int glow_layer = -1;
} config;

constexpr engine::RenderTargetId kColorTarget = 0;
constexpr engine::RenderTargetId kGlowMaskTarget = 1;
constexpr engine::RenderTargetId kGlowBlurTarget = 2;

inline engine::ShaderId glow_mask_shader_id() {
  static const engine::ShaderId id = engine::resources::register_shader("glow_mask_2d");
  return id;
}

inline engine::ShaderId glow_blur_shader_id() {
  static const engine::ShaderId id = engine::resources::register_shader("glow_blur_2d");
  return id;
}

inline engine::ShaderId glow_divide_shader_id() {
  static const engine::ShaderId id = engine::resources::register_shader("glow_divide_2d");
  return id;
}

inline ecs::Entity* make_quad(const glm::vec2& pos, const glm::vec2& size,
                              const engine::UIColor& color, int layer) {
  auto* entity = arena::create<ecs::Entity>();
  auto* transform = arena::create<transform::NoRotationTransform>();
  transform->pos = pos;
  entity->add(transform);
  entity->add(arena::create<layers::ConstLayer>(layer));
  entity->add(arena::create<render_system::QuadRenderable>(size.x, size.y, color));
  entity->add(arena::create<scene::SceneObject>("main"));
  camera_shake::attach(entity, camera_shake::AxisMode::Full, 1.35f);
  return entity;
}

inline glm::vec2 player_center() {
  if (!player::player_transform) return glm::vec2{0.0f, 0.0f};
  return player::player_transform->pos +
         glm::vec2{player::player_size.x * 0.5f, player::player_size.y * 0.5f};
}

struct PlayerGlowRenderable : public render_system::QuadRenderable {
  PlayerGlowRenderable(float width, float height, const engine::UIColor& color)
      : render_system::QuadRenderable(width, height, color) {}

  engine::ShaderId shader_id() const override { return glow_mask_shader_id(); }

  engine::RenderState render_state() const override {
    engine::RenderState state{};
    state.blend = false;
    return state;
  }

  engine::RenderTargetId render_target() const override { return kGlowMaskTarget; }

  void emit(engine::RenderPass& pass) override {
    if (!player::player_transform) return;
    const glm::vec2 center = player_center();
    pass.uniforms.push_back(engine::Uniform{"u_glow_center", engine::Vec2{center.x, center.y}});
    pass.uniforms.push_back(engine::Uniform{"u_glow_radius", radius});
    pass.uniforms.push_back(engine::Uniform{"u_glow_edge", edge});
    pass.uniforms.push_back(engine::Uniform{"u_glow_power", edge_power});
    pass.uniforms.push_back(engine::Uniform{"u_noise_strength", noise_strength});
    pass.uniforms.push_back(engine::Uniform{"u_noise_scale", noise_scale});
    pass.uniforms.push_back(engine::Uniform{"u_mask_strength", mask_strength});
    render_system::QuadRenderable::emit(pass);
  }

  float radius = 0.0f;
  float edge = 0.0f;
  float edge_power = 1.0f;
  float noise_strength = 0.0f;
  float noise_scale = 0.0f;
  float mask_strength = 1.0f;
};

struct GlowQuad {
  ecs::Entity* entity = nullptr;
  transform::NoRotationTransform* transform = nullptr;
  PlayerGlowRenderable* renderable = nullptr;
};

inline GlowQuad glow_quad{};

struct PostQuad {
  engine::GeometryId geometry_id = engine::kInvalidGeometryId;
  engine::GeometryData geometry{};
  bool uploaded = false;
  int width = 0;
  int height = 0;
};

inline PostQuad post_quad{};

inline void ensure_post_quad(int width, int height) {
  if (width <= 0 || height <= 0) return;
  if (post_quad.geometry_id == engine::kInvalidGeometryId) {
    post_quad.geometry_id = engine::resources::register_geometry("shrooms_post_quad");
  }
  if (post_quad.width != width || post_quad.height != height ||
      post_quad.geometry.vertices.empty()) {
    post_quad.geometry = engine::geometry::make_quad(
        static_cast<float>(width), static_cast<float>(height),
        {0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f});
    post_quad.width = width;
    post_quad.height = height;
    post_quad.uploaded = false;
  }
}

inline void emit_post_quad(engine::RenderPass& pass) {
  if (post_quad.geometry_id == engine::kInvalidGeometryId) return;
  if (!post_quad.uploaded) {
    pass.uploads.push_back(engine::GeometryUpload{post_quad.geometry_id, post_quad.geometry});
    post_quad.uploaded = true;
  }
  engine::DrawItem draw{};
  draw.geometry_id = post_quad.geometry_id;
  draw.color = engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f};
  pass.draw_items.push_back(draw);
}

inline void append_post_process(engine::Frame& frame) {
  const int width = shrooms::screen::view_width;
  const int height = shrooms::screen::view_height;
  if (width <= 0 || height <= 0) return;

  frame.plan.targets.reserve(frame.plan.targets.size() + 6);
  frame.plan.targets.push_back(engine::RenderTargetDesc{
      "shrooms_color", width, height, engine::RenderTargetFormat::RGBA8,
      engine::RenderTargetFilter::Linear});
  frame.plan.targets.push_back(engine::RenderTargetDesc{
      "shrooms_glow_mask", width, height, engine::RenderTargetFormat::R8,
      engine::RenderTargetFilter::Linear});
  frame.plan.targets.push_back(engine::RenderTargetDesc{
      "shrooms_glow_blur", width, height, engine::RenderTargetFormat::R8,
      engine::RenderTargetFilter::Linear});
  frame.plan.targets.push_back(engine::RenderTargetDesc{
      "shrooms_mushroom_mask", width, height, engine::RenderTargetFormat::R8,
      engine::RenderTargetFilter::Linear});
  frame.plan.targets.push_back(engine::RenderTargetDesc{
      "shrooms_panel_mask", width, height, engine::RenderTargetFormat::R8,
      engine::RenderTargetFilter::Linear});
  frame.plan.targets.push_back(engine::RenderTargetDesc{
      "shrooms_familiar_panel_mask", width, height, engine::RenderTargetFormat::R8,
      engine::RenderTargetFilter::Linear});

  ensure_post_quad(width, height);

  const engine::Mat4 view = engine::mat4_identity();
  const engine::Mat4 proj = engine::mat4_ortho(0.0f, static_cast<float>(width),
                                               static_cast<float>(height), 0.0f, -1.0f, 1.0f);

  engine::RenderPass clear_glow_mask{};
  clear_glow_mask.name = "glow-mask-clear";
  clear_glow_mask.target = kGlowMaskTarget;
  clear_glow_mask.clear = true;
  clear_glow_mask.clear_color = engine::UIColor{0.0f, 0.0f, 0.0f, 0.0f};

  engine::RenderPass clear_mushroom_mask{};
  clear_mushroom_mask.name = "mushroom-mask-clear";
  clear_mushroom_mask.target = panel_occlusion_fx::kMushroomMaskTarget;
  clear_mushroom_mask.clear = true;
  clear_mushroom_mask.clear_color = engine::UIColor{0.0f, 0.0f, 0.0f, 0.0f};

  engine::RenderPass clear_panel_mask{};
  clear_panel_mask.name = "panel-mask-clear";
  clear_panel_mask.target = panel_occlusion_fx::kPanelMaskTarget;
  clear_panel_mask.clear = true;
  clear_panel_mask.clear_color = engine::UIColor{0.0f, 0.0f, 0.0f, 0.0f};

  engine::RenderPass clear_familiar_panel_mask{};
  clear_familiar_panel_mask.name = "familiar-panel-mask-clear";
  clear_familiar_panel_mask.target = panel_occlusion_fx::kFamiliarPanelMaskTarget;
  clear_familiar_panel_mask.clear = true;
  clear_familiar_panel_mask.clear_color = engine::UIColor{0.0f, 0.0f, 0.0f, 0.0f};

  engine::RenderPass clear_pass{};
  clear_pass.name = "scene-clear";
  clear_pass.target = kColorTarget;
  clear_pass.clear = true;
  clear_pass.clear_color = engine::shrooms::kScreenClearColor;
  frame.plan.passes.insert(frame.plan.passes.begin(), std::move(clear_panel_mask));
  frame.plan.passes.insert(frame.plan.passes.begin(), std::move(clear_familiar_panel_mask));
  frame.plan.passes.insert(frame.plan.passes.begin(), std::move(clear_mushroom_mask));
  frame.plan.passes.insert(frame.plan.passes.begin(), std::move(clear_pass));
  frame.plan.passes.insert(frame.plan.passes.begin(), std::move(clear_glow_mask));

  const engine::Vec2 texel{1.0f / static_cast<float>(width),
                           1.0f / static_cast<float>(height)};

  engine::RenderPass blur_x{};
  blur_x.name = "glow-blur-x";
  blur_x.shader_id = glow_blur_shader_id();
  blur_x.view = view;
  blur_x.proj = proj;
  blur_x.state.blend = false;
  blur_x.target = kGlowBlurTarget;
  blur_x.uniforms.push_back(
      engine::Uniform{"u_blur_tex", engine::RenderTargetRef{kGlowMaskTarget}});
  blur_x.uniforms.push_back(engine::Uniform{"u_texel", texel});
  blur_x.uniforms.push_back(engine::Uniform{"u_direction", engine::Vec2{1.0f, 0.0f}});
  blur_x.uniforms.push_back(engine::Uniform{"u_blur_scale", config.glow_blur_px});
  emit_post_quad(blur_x);
  frame.plan.passes.push_back(std::move(blur_x));

  engine::RenderPass blur_y{};
  blur_y.name = "glow-blur-y";
  blur_y.shader_id = glow_blur_shader_id();
  blur_y.view = view;
  blur_y.proj = proj;
  blur_y.state.blend = false;
  blur_y.target = kGlowMaskTarget;
  blur_y.uniforms.push_back(
      engine::Uniform{"u_blur_tex", engine::RenderTargetRef{kGlowBlurTarget}});
  blur_y.uniforms.push_back(engine::Uniform{"u_texel", texel});
  blur_y.uniforms.push_back(engine::Uniform{"u_direction", engine::Vec2{0.0f, 1.0f}});
  blur_y.uniforms.push_back(engine::Uniform{"u_blur_scale", config.glow_blur_px});
  emit_post_quad(blur_y);
  frame.plan.passes.push_back(std::move(blur_y));

  engine::RenderPass composite{};
  composite.name = "shrooms-composite";
  composite.shader_id = panel_occlusion_fx::composite_shader_id();
  composite.view = view;
  composite.proj = proj;
  composite.state.blend = false;
  composite.target = engine::kRenderTargetBackbuffer;
  composite.uniforms.push_back(engine::Uniform{"u_color_tex", engine::RenderTargetRef{kColorTarget}});
  composite.uniforms.push_back(
      engine::Uniform{"u_glow_mask_tex", engine::RenderTargetRef{kGlowMaskTarget}});
  composite.uniforms.push_back(engine::Uniform{
      "u_mushroom_mask_tex",
      engine::RenderTargetRef{panel_occlusion_fx::kMushroomMaskTarget}});
  composite.uniforms.push_back(engine::Uniform{
      "u_panel_mask_tex", engine::RenderTargetRef{panel_occlusion_fx::kPanelMaskTarget}});
  composite.uniforms.push_back(engine::Uniform{
      "u_familiar_panel_mask_tex",
      engine::RenderTargetRef{panel_occlusion_fx::kFamiliarPanelMaskTarget}});
  composite.uniforms.push_back(engine::Uniform{"u_texel", texel});
  const glm::vec2 screen_shake_offset = game_over_sequence::current_shake_offset();
  composite.uniforms.push_back(engine::Uniform{
      "u_screen_shake_offset",
      engine::Vec2{screen_shake_offset.x, screen_shake_offset.y}});
  composite.uniforms.push_back(engine::Uniform{"u_divide_strength", config.glow_divide_strength});
  composite.uniforms.push_back(engine::Uniform{"u_divide_epsilon", config.glow_divide_epsilon});
  composite.uniforms.push_back(engine::Uniform{
      "u_panel_edge_strength", panel_occlusion_fx::config.edge_strength});
  composite.uniforms.push_back(engine::Uniform{
      "u_panel_edge_color",
      engine::Vec4{panel_occlusion_fx::config.edge_color.x,
                   panel_occlusion_fx::config.edge_color.y,
                   panel_occlusion_fx::config.edge_color.z,
                   panel_occlusion_fx::config.edge_color.w}});
  emit_post_quad(composite);
  frame.plan.passes.push_back(std::move(composite));
}

inline void init() {
  const glm::vec2 view_size{
      static_cast<float>(shrooms::screen::view_width),
      static_cast<float>(shrooms::screen::view_height),
  };

  render_system::set_default_target(kColorTarget);

  const float pad = std::max(config.shake_pad_px, view_size.x * 0.015f);
  const glm::vec2 padded_origin{-pad, -pad};
  const glm::vec2 padded_size{view_size.x + pad * 2.0f, view_size.y + pad * 2.0f};

  const engine::UIColor tint{
      config.color_tint.x, config.color_tint.y, config.color_tint.z, config.color_tint.w};
  make_quad(padded_origin, padded_size, tint, config.tint_layer);

  if (!player::player_transform) return;
  const float player_extent = std::max(player::player_size.x, player::player_size.y);
  const float view_min = std::min(view_size.x, view_size.y);
  const float radius = std::max(player_extent * 0.6f, view_min * config.glow_radius_scale);
  const float edge = std::max(2.0f, view_min * config.glow_edge_scale);

  glow_quad = {};
  glow_quad.entity = arena::create<ecs::Entity>();
  glow_quad.transform = arena::create<transform::NoRotationTransform>();
  glow_quad.transform->pos = padded_origin;
  glow_quad.entity->add(glow_quad.transform);
  glow_quad.entity->add(arena::create<layers::ConstLayer>(config.glow_layer));
  const engine::UIColor glow_color{1.0f, 1.0f, 1.0f, 1.0f};
  glow_quad.renderable =
      arena::create<PlayerGlowRenderable>(padded_size.x, padded_size.y, glow_color);
  glow_quad.renderable->radius = radius;
  glow_quad.renderable->edge = edge;
  glow_quad.renderable->edge_power = config.glow_edge_power;
  glow_quad.renderable->noise_strength = config.glow_noise_strength;
  glow_quad.renderable->noise_scale = config.glow_noise_scale;
  glow_quad.renderable->mask_strength = config.glow_mask_strength;
  glow_quad.entity->add(glow_quad.renderable);
  glow_quad.entity->add(arena::create<scene::SceneObject>("main"));
}

}  // namespace global_fx
