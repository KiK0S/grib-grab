#version 300 es
precision mediump float;

out vec4 o_color;

in vec2 v_uv;

uniform sampler2D u_tex;
uniform vec4 u_color;

void main() {
  float mask = texture(u_tex, v_uv).a * u_color.a;
  o_color = vec4(1.0, 1.0, 1.0, mask);
}
