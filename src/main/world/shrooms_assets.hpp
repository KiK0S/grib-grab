#pragma once

#include <string>

#include "engine/resource_ids.h"
#include "shrooms_texture_sizing.hpp"

namespace shrooms {

inline constexpr float kReferenceCanvasWidthPx = 298.0f;
inline constexpr float kMenuWitchReferenceWidthPx = 120.0f;

inline constexpr float width_ratio_from_reference(float reference_width_px) {
  return reference_width_px > 0.0f ? (reference_width_px / kReferenceCanvasWidthPx) : 0.0f;
}

inline std::string asset_path(const std::string& name) {
#ifdef __EMSCRIPTEN__
  return "/assets/" + name;
#else
  return "../assets/" + name;
#endif
}

inline void register_shrooms_svg_assets() {
  auto register_svg = [](const char* name) {
    const glm::vec2 size = shrooms::texture_sizing::reference_size(name);
    const float width_ratio = width_ratio_from_reference(size.x);
    engine::resources::register_svg_texture(name, width_ratio);
  };
  auto register_svg_with_min_reference_width = [](const char* name,
                                                  float min_reference_width_px) {
    const glm::vec2 size = shrooms::texture_sizing::reference_size(name);
    float reference_width = size.x;
    if (reference_width < min_reference_width_px) {
      reference_width = min_reference_width_px;
    }
    const float width_ratio = width_ratio_from_reference(reference_width);
    engine::resources::register_svg_texture(name, width_ratio);
  };

  register_svg("mukhomor");
  register_svg("lisi4ka");
  register_svg("borovik");
  register_svg("mukhomor_small");
  register_svg("lisi4ka_small");
  register_svg("borovik_small");

  register_svg("witch_left_1");
  register_svg("witch_left_2");
  register_svg("witch_right_1");
  register_svg("witch_right_2");
  // Menu witch is displayed much wider than authored sprite width.
  // Register a larger SVG target to avoid runtime upscaling blur.
  register_svg_with_min_reference_width("witch", kMenuWitchReferenceWidthPx);
  register_svg("face_mini_1");
  register_svg("face_mini_2");
  register_svg("bat_face");
  register_svg("famiriar");
  register_svg("familiar_attack");
  register_svg("digits_1");
  register_svg("digits_2");
  register_svg("digits_3");
  register_svg("menu_pause");
  register_svg("menu_face");
  register_svg("menu_scoreboard");
  register_svg("heart");
  register_svg("background");
  register_svg("bottom_1");
  register_svg("bottom_2");

  register_svg("level_1_ezh");
  register_svg("level_2_eli");
  register_svg("level_3_izba");
  register_svg("level_4_lyaguha");
  register_svg("level_5_mol");
  register_svg("level_6_tzar");
  register_svg("level_7_yagoda");

  register_svg("emoji_hedgehog");
  register_svg("emoji_tree");
  register_svg("emoji_house");
  register_svg("emoji_frog");
  register_svg("emoji_fly");
  register_svg("emoji_crown");
  register_svg("emoji_strawberry");
  register_svg("emoji_infinity");
  register_svg("emoji_lock");
}

}  // namespace shrooms
