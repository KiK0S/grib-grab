#version 300 es
precision mediump float;

out vec4 o_color;

in vec2 v_uv;

uniform sampler2D u_color_tex;
uniform sampler2D u_glow_mask_tex;
uniform sampler2D u_mushroom_mask_tex;
uniform sampler2D u_panel_mask_tex;
uniform vec2 u_texel;
uniform float u_divide_strength;
uniform float u_divide_epsilon;
uniform float u_time;
uniform float u_panel_edge_radius_px;
uniform float u_panel_edge_strength;
uniform float u_panel_pulse_speed;
uniform float u_panel_wave_scale;
uniform vec4 u_panel_edge_color;

float sample_mushroom(vec2 uv) {
  return texture(u_mushroom_mask_tex, clamp(uv, vec2(0.0), vec2(1.0))).r;
}

float mushroom_contour(vec2 uv) {
  float radius = max(u_panel_edge_radius_px, 0.5);
  float center = sample_mushroom(uv);
  float outside = 1.0 - smoothstep(0.08, 0.78, center);

  vec2 dirs[16] = vec2[16](
    vec2( 1.0000,  0.0000),
    vec2( 0.9239,  0.3827),
    vec2( 0.7071,  0.7071),
    vec2( 0.3827,  0.9239),
    vec2( 0.0000,  1.0000),
    vec2(-0.3827,  0.9239),
    vec2(-0.7071,  0.7071),
    vec2(-0.9239,  0.3827),
    vec2(-1.0000,  0.0000),
    vec2(-0.9239, -0.3827),
    vec2(-0.7071, -0.7071),
    vec2(-0.3827, -0.9239),
    vec2( 0.0000, -1.0000),
    vec2( 0.3827, -0.9239),
    vec2( 0.7071, -0.7071),
    vec2( 0.9239, -0.3827)
  );

  float near_mask = 0.0;
  float mid_mask = 0.0;
  float far_mask = 0.0;
  vec2 r = u_texel * radius;
  for (int i = 0; i < 16; ++i) {
    vec2 offset = dirs[i] * r;
    near_mask = max(near_mask, sample_mushroom(uv + offset * 0.45));
    mid_mask = max(mid_mask, sample_mushroom(uv + offset * 0.72));
    far_mask = max(far_mask, sample_mushroom(uv + offset));
  }

  float nearby = max(near_mask, max(mid_mask * 0.72, far_mask * 0.38));
  float exterior_band = nearby * outside;
  return smoothstep(0.04, 0.68, exterior_band);
}

void main() {
  vec4 base = texture(u_color_tex, v_uv);

  float glow_mask = texture(u_glow_mask_tex, v_uv).r;
  float denom = max(1.0 - glow_mask * u_divide_strength, u_divide_epsilon);
  vec3 rgb = clamp(base.rgb / denom, 0.0, 1.0);

  float panel_mask = texture(u_panel_mask_tex, v_uv).r;
  float edge = mushroom_contour(v_uv) * smoothstep(0.10, 0.48, panel_mask);
  float wave = sin(u_time * u_panel_pulse_speed +
                   (v_uv.x * 0.73 + v_uv.y * 1.17) * u_panel_wave_scale);
  float pulse = 0.68 + 0.32 * wave;
  float shimmer = edge * pulse * u_panel_edge_strength;
  vec3 edge_color = u_panel_edge_color.rgb;

  rgb = clamp(rgb + edge_color * shimmer * 0.45, 0.0, 1.0);
  rgb = mix(rgb, max(rgb, edge_color), shimmer * 0.35);
  o_color = vec4(rgb, base.a);
}
