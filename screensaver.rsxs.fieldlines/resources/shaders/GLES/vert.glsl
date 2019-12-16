#version 100

precision mediump float;

// Attributes
attribute vec3 a_pos;
attribute vec4 a_col;

// Uniforms
uniform mat4 u_proj;
uniform mat4 u_model;

// Varyings
varying vec4 v_col;

void main()
{
  gl_Position = u_proj * u_model * vec4(a_pos, 1.0);
  v_col = a_col;
}
