#pragma once

#include <array>
#include <string>
#include <string_view>

#include "engine/resource_ids.h"
#include "shrooms_texture_sizing.hpp"

namespace shrooms {

inline constexpr float kReferenceCanvasWidthPx = 298.0f;
inline constexpr float kMenuWitchReferenceWidthPx = 120.0f;

inline constexpr float width_ratio_from_reference(float reference_width_px) {
  return reference_width_px > 0.0f ? (reference_width_px / kReferenceCanvasWidthPx) : 0.0f;
}

inline constexpr std::array<std::string_view, 6> kMushroomTextureNames{
    "mukhomor",       "lisi4ka",       "borovik",
    "mukhomor_small", "lisi4ka_small", "borovik_small",
};

inline constexpr std::array<std::string_view, 5> kPlayerTextureNames{
    "witch_left_1", "witch_left_2", "witch_right_1", "witch_right_2", "witch",
};

inline constexpr std::array<std::string_view, 5> kFamiliarTextureNames{
    "face_mini_1", "face_mini_2", "bat_face", "famiriar", "familiar_attack",
};

inline constexpr std::array<std::string_view, 8> kUiTextureNames{
    "digits_1", "digits_2",     "digits_3",        "menu_pause",
    "in_game_pause", "menu_face", "menu_scoreboard", "heart",
};

inline constexpr std::array<std::string_view, 10> kBackgroundTextureNames{
    "background",  "bottom_1",    "bottom_2",     "level_1_eli",
    "level_2_ezh", "level_3_izba", "level_4_lyaguha", "level_5_mol",
    "level_6_yagoda", "level_7_tzar",
};

inline constexpr std::array<std::string_view, 9> kEmojiTextureNames{
    "emoji_hedgehog", "emoji_tree",       "emoji_house",
    "emoji_frog",     "emoji_fly",        "emoji_crown",
    "emoji_strawberry", "emoji_infinity", "emoji_lock",
};

inline std::string asset_path(const std::string& name) {
#ifdef __EMSCRIPTEN__
  return "/assets/" + name;
#else
  return "../assets/" + name;
#endif
}

inline void register_shrooms_svg_assets() {
  auto register_svg = [](std::string_view name) {
    const glm::vec2 size = shrooms::texture_sizing::reference_size(name);
    const float width_ratio = width_ratio_from_reference(size.x);
    engine::resources::register_svg_texture(std::string{name}, width_ratio);
  };
  auto register_svg_with_min_reference_width = [](std::string_view name,
                                                  float min_reference_width_px) {
    const glm::vec2 size = shrooms::texture_sizing::reference_size(name);
    float reference_width = size.x;
    if (reference_width < min_reference_width_px) {
      reference_width = min_reference_width_px;
    }
    const float width_ratio = width_ratio_from_reference(reference_width);
    engine::resources::register_svg_texture(std::string{name}, width_ratio);
  };

  for (std::string_view name : kMushroomTextureNames) register_svg(name);
  for (std::string_view name : kPlayerTextureNames) {
    if (name == "witch") {
      // Menu witch is displayed much wider than authored sprite width.
      // Register a larger SVG target to avoid runtime upscaling blur.
      register_svg_with_min_reference_width(name, kMenuWitchReferenceWidthPx);
    } else {
      register_svg(name);
    }
  }
  for (std::string_view name : kFamiliarTextureNames) register_svg(name);
  for (std::string_view name : kUiTextureNames) register_svg(name);
  for (std::string_view name : kBackgroundTextureNames) register_svg(name);
  for (std::string_view name : kEmojiTextureNames) register_svg(name);
}

}  // namespace shrooms
