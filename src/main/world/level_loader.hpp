#pragma once

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "glm/glm/vec2.hpp"

#include "ecs/ecs.hpp"
#include "utils/arena.hpp"
#include "utils/random.hpp"
#include "utils/file_system.hpp"

#include "systems/geometry/shapes/quad.hpp"
#include "systems/geometry/shapes/polygon.hpp"
#include "systems/transformation/transform_object.hpp"
#include "systems/animation/sprite_animation.hpp"
#include "systems/render/sprite_system.hpp"
#include "systems/render/render_system.hpp"
#include "systems/color/color_system.hpp"
#include "systems/layer/layered_object.hpp"
#include "systems/collision/collider_object.hpp"
#include "systems/spawning/periodic_spawner_system.hpp"
#include "systems/spawn/periodic_spawner_object.hpp"
#include "systems/moving/moving_object.hpp"
#include "systems/rotating/rotating_object.hpp"
#include "systems/hidden/hidden_object.hpp"
#include "systems/defer/deferred_system.hpp"
#include "engine/resource_ids.h"
#include "systems/scene/scene_object.hpp"

#include "shrooms_assets.hpp"
#include "shrooms_screen.hpp"
#include "shrooms_texture_sizing.hpp"
#include "ambient_layers.hpp"
#include "vfx.hpp"

namespace levels {
void register_spawner(periodic_spawn::PeriodicSpawnerObject* spawner);
void register_background_sprite(render_system::SpriteRenderable* sprite,
                                transform::NoRotationTransform* transform,
                                const std::string& texture_name);
void on_mushroom_spawned(const std::string& type, ecs::Entity* entity);
}  // namespace levels

namespace level_loader {

struct GeometryParseResult {
  geometry::GeometryObject* geometry = nullptr;
  glm::vec2 min{0.0f, 0.0f};
  glm::vec2 max{0.0f, 0.0f};
};

inline constexpr int kSpawnWarningMs = 350;
inline constexpr bool kEnableSpawnRuleLogging = false;

inline std::string parse_entity_description(std::istream& in) {
  std::string name;
  in >> name;
  std::string res = name;
  std::string token;
  while (in >> token) {
    res = res + ' ' + token;
    if (token == name) {
      break;
    }
  }
  return res;
}

inline GeometryParseResult parse_geometry(std::istream& in, const std::string& name) {
  std::string type;
  in >> type;

  int n = 0;
  if (type == "quad") {
    n = 4;
  } else {
    in >> n;
  }

#ifndef NDEBUG
  if (type != "quad" && type != "polygon") {
    std::fprintf(stderr, "Unknown geometry type '%s' for %s\n", type.c_str(), name.c_str());
  }
#endif

  std::vector<glm::vec2> points;
  points.reserve(n);
  for (int i = 0; i < n; ++i) {
    glm::vec2 p{};
    in >> p.x >> p.y;
    points.push_back(shrooms::screen::norm_to_pixels(p));
  }

  GeometryParseResult result{};
  if (points.empty()) {
    return result;
  }

  glm::vec2 min = points[0];
  glm::vec2 max = points[0];
  for (const auto& p : points) {
    min.x = std::min(min.x, p.x);
    min.y = std::min(min.y, p.y);
    max.x = std::max(max.x, p.x);
    max.y = std::max(max.y, p.y);
  }

  std::vector<glm::vec2> local_points;
  local_points.reserve(points.size());
  for (const auto& p : points) {
    local_points.push_back(p - min);
  }

  const bool quad_ok = (type == "quad" && local_points.size() >= 4);
  const bool poly_ok = (type == "polygon" && local_points.size() >= 3);
  if (!quad_ok && !poly_ok && type != "polygon") {
#ifndef NDEBUG
    std::fprintf(stderr, "Invalid geometry '%s' for %s points=%zu\n", type.c_str(),
                 name.c_str(), local_points.size());
#endif
    return result;
  }

  result.min = min;
  result.max = max;
  if (type == "polygon") {
    result.geometry = arena::create<geometry::Polygon>(name, local_points);
  } else if (quad_ok) {
    result.geometry = arena::create<geometry::Quad>(name, local_points);
  }
  return result;
}

inline float parse_float(std::istream& in) {
  std::string token;
  in >> token;
  if (token == "random") {
    float a = 0.0f;
    float b = 0.0f;
    in >> a >> b;
    return static_cast<float>(rnd::get_double(a, b));
  }
  return std::stof(token);
}

inline dynamic::MovingObject* parse_moving(std::istream& in) {
  constexpr float kAuthoredTicksPerSecond = 60.0f;
  glm::vec2 point{};
  in >> point.x >> point.y;
  glm::vec2 px = shrooms::screen::scale_to_pixels(glm::vec2{point.x, -point.y} * 0.5f);
  return arena::create<dynamic::MovingObject>(px * kAuthoredTicksPerSecond);
}

inline dynamic::RotatingObject* parse_rotating(std::istream& in) {
  float angle = parse_float(in);
  return arena::create<dynamic::RotatingObject>(angle);
}

inline std::string parse_texture(std::istream& in) {
  std::string filepath;
  in >> filepath;
  return filepath;
}

inline layers::LayeredObject* parse_layer(std::istream& in) {
  int layer_num = 0;
  in >> layer_num;
  return arena::create<layers::ConstLayer>(layer_num);
}

inline collision::ColliderObject* parse_collider(std::istream& in) {
  std::string handler_name;
  in >> handler_name;
  return arena::create<collision::ColliderObject>(handler_name);
}

inline collision::TriggerObject* parse_trigger(std::istream& in) {
  std::string handler_name;
  in >> handler_name;
  auto callback = collision::TriggerCallbackRegistry::get_callback(handler_name);
  if (!callback) {
    callback = [](ecs::Entity*, collision::ColliderObject*) {};
  }
  return arena::create<collision::TriggerObject>(handler_name, callback);
}

inline color::ColoredObject* parse_color(std::istream& in) {
  int r = 255;
  int g = 255;
  int b = 255;
  int a = 255;
  in >> r >> g >> b >> a;
  return arena::create<color::OneColor>(
      glm::vec4{r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f});
}

inline std::string extract_texture_name(const std::string& desc) {
  std::istringstream stream(desc);
  std::string token;
  if (!(stream >> token)) {
    return "";
  }
  while (stream >> token) {
    if (token == "texture") {
      std::string texture_name;
      stream >> texture_name;
      return texture_name;
    }
  }
  return "";
}

inline ecs::Entity* parse_entity(std::istream& in);

inline periodic_spawn::PeriodicSpawnerObject* parse_periodic_spawner(std::istream& in) {
  float period = 0.0f;
  double density = 0.0;
  in >> period >> density;

  const std::string desc = parse_entity_description(in);
  const std::string texture_name = extract_texture_name(desc);
  LOG_IF(kEnableSpawnRuleLogging,
         "Spawner parse: period=" << period << " density=" << density
                                  << " type=" << texture_name << " desc=[" << desc
                                  << "]");
  const double scaled_density = density;

  auto* spawner = arena::create<periodic_spawn::PeriodicSpawnerObject>(
      period,
      spawn::SpawningRule{
          scaled_density,
          [=](glm::vec2 pos) {
            LOG_IF(kEnableSpawnRuleLogging,
                   "Spawn rule: type=" << texture_name << " pos=(" << pos.x
                                       << ", " << pos.y << ")");
            std::istringstream new_desc(desc);
            auto* new_entity = parse_entity(new_desc);
            if (!new_entity) {
              LOG_IF(kEnableSpawnRuleLogging, "Spawn rule: failed to parse entity desc");
              return static_cast<ecs::Entity*>(nullptr);
            }

            const glm::vec2 size =
                shrooms::texture_sizing::from_reference_width(texture_name, 28.0f);
            auto* transform = new_entity->get<transform::NoRotationTransform>();
            if (!transform) {
              transform = arena::create<transform::NoRotationTransform>();
              new_entity->add(transform);
            }
            transform->pos = shrooms::screen::center_to_top_left(pos, size);

            new_entity->add(arena::create<geometry::Quad>(
                "spawned_quad",
                std::vector<glm::vec2>{
                    glm::vec2{0.0f, 0.0f},
                    glm::vec2{size.x, 0.0f},
                    glm::vec2{0.0f, size.y},
                    glm::vec2{size.x, size.y},
                }));

            if (!texture_name.empty()) {
              const engine::TextureId tex_id =
                  engine::resources::register_texture(texture_name);
              new_entity->add(arena::create<render_system::SpriteRenderable>(tex_id, size));
            }

            if (auto* geom = new_entity->get<geometry::GeometryObject>()) {
              LOG_IF(kEnableSpawnRuleLogging,
                     "Spawn rule: geometry=" << geom->get_name()
                                             << " size=" << geom->get_size());
            } else {
              LOG_IF(kEnableSpawnRuleLogging, "Spawn rule: missing geometry component");
            }

            new_entity->add(arena::create<scene::SceneObject>("main"));
            vfx::spawn_spawn_warning(
                pos, size, static_cast<float>(kSpawnWarningMs) / 1000.0f);

            auto* hidden = arena::create<hidden::HiddenObject>();
            hidden->hide();
            new_entity->add(hidden);

            glm::vec2 original_translate{0.0f, 0.0f};
            if (auto* moving = new_entity->get<dynamic::MovingObject>()) {
              original_translate = moving->translate;
              moving->translate = glm::vec2{0.0f, 0.0f};
            }

            deferred::fire_deferred(
                [new_entity, pos, size, original_translate]() {
                  if (!new_entity || new_entity->is_pending_deletion()) return;
                  if (auto* hidden = new_entity->get<hidden::HiddenObject>()) {
                    hidden->show();
                  }
                  if (auto* moving = new_entity->get<dynamic::MovingObject>()) {
                    moving->translate = original_translate;
                  }
                  vfx::spawn_spawn_effect(pos, size);
                },
                kSpawnWarningMs);
            levels::on_mushroom_spawned(texture_name, new_entity);
            return new_entity;
          }},
      texture_name);

  spawner->enabled = false;
  spawner->max_spawn_count = 0;
  spawner->spawned_count = 0;
  levels::register_spawner(spawner);
  return spawner;
}

inline ecs::Entity* parse_entity(std::istream& in) {
  std::string name;
  if (!(in >> name)) return nullptr;

  auto* e = arena::create<ecs::Entity>();
  std::string comp;
  GeometryParseResult geom{};
  bool has_geometry = false;
  glm::vec2 size{0.0f, 0.0f};
  std::string texture_name{};
  render_system::SpriteRenderable* sprite = nullptr;
  transform::NoRotationTransform* transform = nullptr;

  while (in >> comp) {
    if (comp == name) break;
    if (comp == "geometry") {
      geom = parse_geometry(in, name);
      if (geom.geometry) {
        e->add(geom.geometry);
        has_geometry = true;
      }
    } else if (comp == "texture") {
      texture_name = parse_texture(in);
    } else if (comp == "color") {
      e->add(parse_color(in));
    } else if (comp == "layer") {
      e->add(parse_layer(in));
    } else if (comp == "moving") {
      e->add(parse_moving(in));
    } else if (comp == "rotating") {
      e->add(parse_rotating(in));
    } else if (comp == "collider") {
      e->add(parse_collider(in));
    } else if (comp == "trigger") {
      e->add(parse_trigger(in));
    } else if (comp == "periodic_spawner") {
      e->add(parse_periodic_spawner(in));
    }
  }

  if (has_geometry) {
    size = geom.max - geom.min;
    transform = arena::create<transform::NoRotationTransform>();
    transform->pos = geom.min;
    e->add(transform);

    if (!texture_name.empty()) {
      if (name == "background") {
        size = shrooms::texture_sizing::from_width_px(
            texture_name, static_cast<float>(shrooms::screen::view_width));
        transform->pos = glm::vec2{
            0.0f,
            static_cast<float>(shrooms::screen::view_height) - size.y,
        };
      }
      const engine::TextureId tex_id =
          engine::resources::register_texture(texture_name);
      sprite = arena::create<render_system::SpriteRenderable>(tex_id, size);
      e->add(sprite);
    } else if (auto* colored = e->get<color::ColoredObject>()) {
      const auto c = colored->get_color();
      e->add(arena::create<render_system::QuadRenderable>(
          size.x, size.y, engine::UIColor{c.x, c.y, c.z, c.w}));
    }
  }

  if (has_geometry) {
    if (name == "background") {
      const glm::vec2 amplitude{
          std::max(2.0f, size.x * 0.004f),
          std::max(2.0f, size.y * 0.004f),
      };
      vfx::attach_wobble(e, amplitude, 0.12f);
      if (sprite) {
        levels::register_background_sprite(sprite, transform, texture_name);
      }
    } else if (name == "floor" && sprite) {
      const engine::TextureId frame_1 = engine::resources::register_texture("bottom_1");
      const engine::TextureId frame_2 = engine::resources::register_texture("bottom_2");
      std::map<std::string, std::vector<animation::SpriteFrame>> clips{};
      clips["idle"] = {animation::SpriteFrame{frame_1, 0.25f},
                       animation::SpriteFrame{frame_2, 0.25f}};
      e->add(arena::create<animation::SpriteAnimation>(std::move(clips), "idle"));
      ambient_layers::register_bottom_sprite(e);
    } else if (name.find("_spawned") != std::string::npos) {
      const glm::vec2 amplitude{size.x * 0.06f, size.y * 0.05f};
      const float speed = static_cast<float>(rnd::get_double(1.4, 2.4));
      vfx::attach_wobble(e, amplitude, speed);
    }
  }

  return e;
}

inline std::vector<ecs::Entity*> parse(std::istream& in) {
  std::vector<ecs::Entity*> res;
  ecs::Entity* e = nullptr;
  while ((e = parse_entity(in)) != nullptr) {
    res.push_back(e);
  }
  return res;
}

inline std::vector<ecs::Entity*> load(const std::string& filename) {
  std::ifstream in(filename);
  if (!in.is_open()) {
    std::cerr << "Failed to open level config: " << filename << std::endl;
    return {};
  }

  auto entities = parse(in);
  for (auto* e : entities) {
    if (!e) continue;
    e->add(arena::create<scene::SceneObject>("main"));
  }
  return entities;
}

inline std::vector<ecs::Entity*> load_default() {
  return load(shrooms::asset_path("mushrooms.data"));
}

}  // namespace level_loader
