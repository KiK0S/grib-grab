#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

#include "glm/glm/vec2.hpp"
#include "glm/glm/vec4.hpp"

#include "ecs/ecs.hpp"
#include "engine/geometry_builder.h"
#include "engine/resource_ids.h"
#include "utils/arena.hpp"
#include "systems/render/renderable.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/transformation/transform_object.hpp"

namespace panel_occlusion_fx {

inline constexpr engine::RenderTargetId kMushroomMaskTarget = 3;
inline constexpr engine::RenderTargetId kPanelMaskTarget = 4;

struct Config {
  float edge_radius_px = 3.0f;
  float edge_strength = 0.62f;
  float pulse_speed = 5.5f;
  float wave_scale = 85.0f;
  glm::vec4 edge_color{1.0f, 0.92f, 0.56f, 1.0f};
} config;

inline engine::ShaderId alpha_mask_shader_id() {
  static const engine::ShaderId id = engine::resources::register_shader("alpha_mask_2d");
  return id;
}

inline engine::ShaderId composite_shader_id() {
  static const engine::ShaderId id = engine::resources::register_shader("shrooms_composite_2d");
  return id;
}

struct AlphaMaskRenderable : public render_system::Renderable {
  AlphaMaskRenderable(engine::TextureId texture_id, glm::vec2 size,
                      engine::RenderTargetId target)
      : texture_id(texture_id), size(size), target(target) {
    geometry_id = engine::resources::register_geometry(
        "alpha_mask_" + std::to_string(reinterpret_cast<uintptr_t>(this)));
    geometry = engine::geometry::make_quad(size.x, size.y);
  }
  ~AlphaMaskRenderable() override { Component::component_count--; }

  engine::ShaderId shader_id() const override { return alpha_mask_shader_id(); }
  engine::RenderTargetId render_target() const override { return target; }

  void set_source(engine::TextureId next_texture_id, glm::vec2 next_size) {
    texture_id = next_texture_id;
    if (std::abs(size.x - next_size.x) < 0.5f && std::abs(size.y - next_size.y) < 0.5f) {
      return;
    }
    size = next_size;
    geometry = engine::geometry::make_quad(size.x, size.y);
    uploaded = false;
  }

  void emit(engine::RenderPass& pass) override {
    if (!entity || entity->is_pending_deletion()) return;
    auto* transform = entity->get<transform::TransformObject>();
    if (!transform) return;

    const glm::vec2 pos = transform->get_pos();
    engine::DrawItem item{};
    if (!uploaded) {
      pass.uploads.push_back(engine::GeometryUpload{geometry_id, geometry});
      uploaded = true;
    }
    item.geometry_id = geometry_id;
    item.model = engine::mat4_translate(
        pos.x + render_system::view_offset_x, pos.y + render_system::view_offset_y, 0.0f);
    item.color = engine::UIColor{1.0f, 1.0f, 1.0f, 1.0f};
    item.texture_id = texture_id;
    pass.draw_items.push_back(item);
  }

  engine::TextureId texture_id = engine::kInvalidTextureId;
  glm::vec2 size{0.0f, 0.0f};
  engine::RenderTargetId target = engine::kInvalidRenderTargetId;
  engine::GeometryId geometry_id = engine::kInvalidGeometryId;
  engine::GeometryData geometry{};
  bool uploaded = false;
};

inline AlphaMaskRenderable* attach_alpha_mask(ecs::Entity* entity,
                                              engine::TextureId texture_id,
                                              const glm::vec2& size,
                                              engine::RenderTargetId target) {
  if (!entity || texture_id == engine::kInvalidTextureId || size.x <= 0.0f || size.y <= 0.0f) {
    return nullptr;
  }
  auto* mask = arena::create<AlphaMaskRenderable>(texture_id, size, target);
  entity->add(mask);
  return mask;
}

inline AlphaMaskRenderable* attach_mushroom_mask(ecs::Entity* entity) {
  if (!entity) return nullptr;
  if (entity->get<AlphaMaskRenderable>()) return nullptr;
  auto* sprite = entity->get<render_system::SpriteRenderable>();
  if (!sprite) return nullptr;
  return attach_alpha_mask(entity, sprite->texture_id, sprite->size, kMushroomMaskTarget);
}

}  // namespace panel_occlusion_fx
