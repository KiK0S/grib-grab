#pragma once

#include <algorithm>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "glm/glm/vec2.hpp"

#include "shrooms_screen.hpp"
#include "utils/file_system.hpp"

namespace shrooms::texture_sizing {

inline constexpr float kReferenceCanvasWidthPx = 298.0f;

inline constexpr float width_scale_from_reference(float reference_width_px) {
  return reference_width_px > 0.0f ? (reference_width_px / kReferenceCanvasWidthPx) : 0.0f;
}

inline std::optional<glm::vec2> parse_viewbox_size(const std::string& svg_text) {
  const size_t key_pos = svg_text.find("viewBox");
  if (key_pos == std::string::npos) return std::nullopt;
  const size_t eq_pos = svg_text.find('=', key_pos);
  if (eq_pos == std::string::npos) return std::nullopt;
  const size_t quote_start = svg_text.find_first_of("\"'", eq_pos + 1);
  if (quote_start == std::string::npos) return std::nullopt;
  const char quote = svg_text[quote_start];
  const size_t quote_end = svg_text.find(quote, quote_start + 1);
  if (quote_end == std::string::npos) return std::nullopt;

  std::string value = svg_text.substr(quote_start + 1, quote_end - quote_start - 1);
  for (char& ch : value) {
    if (ch == ',') ch = ' ';
  }

  std::stringstream in(value);
  float min_x = 0.0f;
  float min_y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
  if (!(in >> min_x >> min_y >> width >> height)) return std::nullopt;
  if (width <= 0.0f || height <= 0.0f) return std::nullopt;
  return glm::vec2{width, height};
}

inline std::optional<glm::vec2> load_svg_reference_size(std::string_view texture_name) {
  if (texture_name.empty()) return std::nullopt;
  const std::string path = file::asset("shrooms/" + std::string(texture_name) + ".svg");
  std::ifstream input(path.c_str());
  if (!input.is_open()) return std::nullopt;

  std::stringstream buffer;
  buffer << input.rdbuf();
  return parse_viewbox_size(buffer.str());
}

inline glm::vec2 reference_size(std::string_view texture_name) {
  static std::unordered_map<std::string, glm::vec2> cached_sizes{};
  static std::unordered_set<std::string> missing{};

  const std::string key(texture_name);
  if (key.empty()) return glm::vec2{0.0f, 0.0f};

  if (auto it = cached_sizes.find(key); it != cached_sizes.end()) {
    return it->second;
  }
  if (missing.contains(key)) {
    return glm::vec2{0.0f, 0.0f};
  }

  if (auto loaded = load_svg_reference_size(key)) {
    cached_sizes.emplace(key, *loaded);
    return *loaded;
  }

  missing.insert(key);
  return glm::vec2{0.0f, 0.0f};
}

inline float aspect_ratio(std::string_view texture_name) {
  const glm::vec2 measured = reference_size(texture_name);
  if (measured.x > 0.0f && measured.y > 0.0f) {
    return measured.x / measured.y;
  }

  if (texture_name == "famiriar") return 29.0f / 14.0f;

  if (texture_name == "witch") return 35.0f / 39.0f;
  if (texture_name == "witch_left_1" || texture_name == "witch_left_2" ||
      texture_name == "witch_right_1" || texture_name == "witch_right_2") {
    return 38.0f / 39.0f;
  }
  if (texture_name == "witch_fly_left_1" || texture_name == "witch_fly_left_2" ||
      texture_name == "witch_fly_right_1" || texture_name == "witch_fly_right_2") {
    return 62.0f / 41.0f;
  }

  if (texture_name == "mukhomor") return 28.0f / 26.0f;
  if (texture_name == "lisi4ka") return 25.0f / 24.0f;
  if (texture_name == "borovik") return 27.0f / 26.0f;
  if (texture_name == "mukhomor_small") return 16.0f / 15.0f;
  if (texture_name == "lisi4ka_small") return 15.0f / 14.0f;
  if (texture_name == "borovik_small") return 16.0f / 16.0f;

  if (texture_name == "menu_face") return 67.0f / 63.0f;
  if (texture_name == "menu_scoreboard") return 64.0f / 88.0f;
  if (texture_name == "face_mini_1" || texture_name == "face_mini_2") {
    return 44.0f / 24.0f;
  }
  if (texture_name == "bat_face") return 180.0f / 154.0f;

  if (texture_name == "level_1_eli") return 298.0f / 290.0f;
  if (texture_name == "level_2_ezh") return 298.0f / 255.0f;
  if (texture_name == "level_3_izba") return 298.0f / 266.0f;
  if (texture_name == "level_4_lyaguha") return 298.0f / 287.0f;
  if (texture_name == "level_5_mol") return 312.0f / 263.0f;
  if (texture_name == "level_6_yagoda") return 298.0f / 281.0f;
  if (texture_name == "level_7_tzar") return 298.0f / 277.0f;

  if (texture_name == "bottom_1") return 298.0f / 20.0f;
  if (texture_name == "bottom_2") return 298.0f / 23.0f;

  if (texture_name == "bullet") return 1.0f;
  if (texture_name == "pause_menu" || texture_name == "menu_pause") return 1.0f;
  if (texture_name == "fire") return 1.0f;
  if (texture_name == "explosion") return 1.0f;
  if (texture_name == "slash") return 1.0f;
  if (texture_name.size() == 8 && texture_name.substr(0, 7) == "digits_") return 1.0f;

  return 1.0f;
}

inline glm::vec2 from_width_px(std::string_view texture_name, float width_px) {
  const float safe_width = width_px > 0.0f ? width_px : 0.0f;
  const float aspect = aspect_ratio(texture_name);
  const float safe_aspect = aspect > 0.0f ? aspect : 1.0f;
  return glm::vec2{safe_width, safe_width / safe_aspect};
}

inline glm::vec2 from_width_scale(std::string_view texture_name, float width_scale) {
  const float width_px =
      width_scale * static_cast<float>(shrooms::screen::view_width);
  return from_width_px(texture_name, width_px);
}

inline glm::vec2 from_reference_width(std::string_view texture_name, float reference_width_px) {
  return from_width_scale(texture_name, width_scale_from_reference(reference_width_px));
}

inline glm::vec2 reference_size_from_width(std::string_view texture_name,
                                           float reference_width_px) {
  const float safe_width = reference_width_px > 0.0f ? reference_width_px : 0.0f;
  const float aspect = aspect_ratio(texture_name);
  const float safe_aspect = aspect > 0.0f ? aspect : 1.0f;
  return glm::vec2{safe_width, safe_width / safe_aspect};
}

inline glm::vec2 from_reference_size(const glm::vec2& reference_size_px) {
  const float scale = 1.0f / kReferenceCanvasWidthPx;
  const float unit = static_cast<float>(shrooms::screen::view_width) * scale;
  return glm::vec2{
      std::max(reference_size_px.x, 0.0f) * unit,
      std::max(reference_size_px.y, 0.0f) * unit,
  };
}

}  // namespace shrooms::texture_sizing
