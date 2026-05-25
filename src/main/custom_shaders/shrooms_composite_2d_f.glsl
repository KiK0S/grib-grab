#version 300 es
precision mediump float;

out vec4 o_color;

in vec2 v_uv;

uniform sampler2D u_color_tex;
uniform sampler2D u_glow_mask_tex;
uniform sampler2D u_mushroom_mask_tex;
uniform sampler2D u_panel_mask_tex;
uniform sampler2D u_familiar_panel_mask_tex;
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

float mushroom_edge(vec2 uv) {
  float radius = max(u_panel_edge_radius_px, 0.5);
  vec2 r = u_texel * radius;
  vec2 h = r * 0.55;

  float center = sample_mushroom(uv);
  float hi = center;
  float lo = center;

  vec2 offsets[12] = vec2[12](
    vec2( r.x, 0.0),
    vec2(-r.x, 0.0),
    vec2(0.0,  r.y),
    vec2(0.0, -r.y),
    vec2( r.x,  r.y),
    vec2(-r.x,  r.y),
    vec2( r.x, -r.y),
    vec2(-r.x, -r.y),
    vec2( h.x,  h.y),
    vec2(-h.x,  h.y),
    vec2( h.x, -h.y),
    vec2(-h.x, -h.y)
  );

  for (int i = 0; i < 12; ++i) {
    float s = sample_mushroom(uv + offsets[i]);
    hi = max(hi, s);
    lo = min(lo, s);
  }

  return smoothstep(0.04, 0.34, hi - lo);
}

void main() {
  vec4 base = texture(u_color_tex, v_uv);

  float glow_mask = texture(u_glow_mask_tex, v_uv).r;
  float denom = max(1.0 - glow_mask * u_divide_strength, u_divide_epsilon);
  vec3 rgb = clamp(base.rgb / denom, 0.0, 1.0);

  float panel_mask = texture(u_panel_mask_tex, v_uv).r;
  float edge = mushroom_edge(v_uv) * smoothstep(0.08, 0.42, panel_mask);
  float familiar_panel_mask = texture(u_familiar_panel_mask_tex, v_uv).r *
                              smoothstep(0.08, 0.42, panel_mask);
  float wave = sin(u_time * u_panel_pulse_speed +
                   (v_uv.x * 0.73 + v_uv.y * 1.17) * u_panel_wave_scale);
  float pulse = 0.68 + 0.32 * wave;
  float shimmer = edge * pulse * u_panel_edge_strength;
  vec3 edge_color = u_panel_edge_color.rgb;

  rgb = clamp(rgb + edge_color * shimmer * 0.45, 0.0, 1.0);
  rgb = mix(rgb, max(rgb, edge_color), shimmer * 0.35);
  float familiar_visibility =
      familiar_panel_mask * clamp(u_panel_edge_strength * 0.32, 0.0, 0.42);
  rgb = mix(rgb, max(rgb, edge_color), familiar_visibility);
  o_color = vec4(rgb, base.a);
}
