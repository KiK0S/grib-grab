#pragma once

#include <cmath>

#include "glm/glm/vec2.hpp"

namespace shrooms::pixel_snap {

inline float coord(float value) { return std::round(value); }

inline glm::vec2 point(const glm::vec2& value) {
  return glm::vec2{coord(value.x), coord(value.y)};
}

inline glm::vec2 centered_top_left(const glm::vec2& center, const glm::vec2& size) {
  return point(center - size * 0.5f);
}

}  // namespace shrooms::pixel_snap
