#version 300 es
precision mediump float;

out vec4 o_color;

in vec2 v_uv;
in vec4 v_color;

uniform sampler2D u_tex;
uniform vec4 u_color;

#define MAX_POINTS 12
uniform float u_point_count;
uniform vec2 u_from[MAX_POINTS];
uniform vec2 u_to[MAX_POINTS];
uniform float u_radius[MAX_POINTS];
uniform float u_warp_power;
uniform float u_warp_epsilon;
uniform float u_warp_rest_weight;

vec2 compute_delta(vec2 uv) {
  int count = int(floor(u_point_count + 0.5));
  vec2 delta_sum = vec2(0.0);
  float weight_sum = 0.0;
  for (int i = 0; i < MAX_POINTS; ++i) {
    if (i >= count) break;
    vec2 from = u_from[i];
    vec2 delta = u_to[i] - from;
    float radius = max(u_radius[i], u_warp_epsilon);
    if (radius <= 0.0) continue;
    float dist = length(uv - from);
    float d = dist / radius;
    float w = 1.0 / (1.0 + pow(max(d, 0.0), max(1.0, u_warp_power)));
    delta_sum += delta * w;
    weight_sum += w;
  }
  float denom = max(u_warp_rest_weight, 0.0) + weight_sum;
  if (denom <= 0.0) return vec2(0.0);
  return delta_sum / denom;
}

void main() {
  vec2 warped_uv = v_uv - compute_delta(v_uv);
  if (warped_uv.x < 0.0 || warped_uv.x > 1.0 ||
      warped_uv.y < 0.0 || warped_uv.y > 1.0) {
    o_color = vec4(0.0);
    return;
  }

  float mask = texture(u_tex, clamp(warped_uv, vec2(0.0), vec2(1.0))).a *
               v_color.a * u_color.a;
  o_color = vec4(1.0, 1.0, 1.0, mask);
}
