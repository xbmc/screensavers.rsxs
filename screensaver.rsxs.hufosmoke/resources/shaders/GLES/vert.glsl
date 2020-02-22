#version 100

#ifdef GL_ES
precision mediump float;
#endif

// Attributes
attribute vec4 a_position;
attribute vec4 a_color;

// Uniforms
uniform mat4 u_modelViewProjectionMatrix;

// Varyings
varying vec4 v_frontColor;

void main ()
{
  gl_Position = u_modelViewProjectionMatrix * a_position;

  v_frontColor = a_color;
}
