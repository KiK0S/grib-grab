#version 300 es
in vec3 a_pos;
in vec3 a_normal;
in vec2 a_uv;
in vec4 a_color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec2 v_uv;
out vec4 v_color;

void main() {
  gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);
  v_uv = a_uv;
  v_color = a_color;
}
