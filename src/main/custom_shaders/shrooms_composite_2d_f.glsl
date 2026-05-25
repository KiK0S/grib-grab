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
uniform vec2 u_screen_shake_offset;
uniform float u_divide_strength;
uniform float u_divide_epsilon;
uniform float u_panel_edge_strength;
uniform vec4 u_panel_edge_color;

float sample_mushroom(vec2 uv) {
  return texture(u_mushroom_mask_tex, clamp(uv, vec2(0.0), vec2(1.0))).r;
}

float outside_original_screen(vec2 uv) {
  vec2 original_uv = uv - u_screen_shake_offset * u_texel;
  return (original_uv.x < 0.0 || original_uv.x > 1.0 ||
          original_uv.y < 0.0 || original_uv.y > 1.0) ? 1.0 : 0.0;
}

float panel_coverage_at(vec2 uv) {
  float panel_mask = texture(u_panel_mask_tex, clamp(uv, vec2(0.0), vec2(1.0))).r;
  return max(smoothstep(0.08, 0.42, panel_mask), outside_original_screen(uv));
}

void main() {
  vec4 base = texture(u_color_tex, v_uv);

  float glow_mask = texture(u_glow_mask_tex, v_uv).r;
  float denom = max(1.0 - glow_mask * u_divide_strength, u_divide_epsilon);
  vec3 rgb = clamp(base.rgb / denom, 0.0, 1.0);

  float panel_coverage = panel_coverage_at(v_uv);
  float mushroom_panel_mask = sample_mushroom(v_uv) * panel_coverage;
  float familiar_panel_mask = texture(u_familiar_panel_mask_tex, v_uv).r *
                              panel_coverage;
  float mask_visibility =
      max(mushroom_panel_mask, familiar_panel_mask) *
      clamp(u_panel_edge_strength * 0.32, 0.0, 0.42);
  rgb = mix(rgb, max(rgb, u_panel_edge_color.rgb), mask_visibility);
  o_color = vec4(rgb, base.a);
}
