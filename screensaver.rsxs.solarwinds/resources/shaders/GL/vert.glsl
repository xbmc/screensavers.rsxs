#version 130

struct Light {
  vec4 position;
  vec4 ambient;
  vec4 diffuse;
};

// Attributes
attribute vec4 a_position;
attribute vec4 a_color;
attribute vec2 a_coord;

// Uniforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform int u_geometry;

// Varyings
smooth out vec4 v_frontColor;
out vec2 v_coord;
out vec4 v_position;

void main ()
{
  gl_Position = u_projectionMatrix * u_modelViewMatrix * a_position;

  v_frontColor = a_color;
  v_coord = a_coord;
  v_position = a_position;
}
