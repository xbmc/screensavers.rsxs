#version 100

precision mediump float;

// Attributes
attribute vec4 a_position;
attribute vec4 a_color;

// Uniforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;

// Varyings
varying vec4 v_frontColor;

void main ()
{
  gl_Position = u_projectionMatrix * u_modelViewMatrix * a_position;

  v_frontColor = a_color;
}
