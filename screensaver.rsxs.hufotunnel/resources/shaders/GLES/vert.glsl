#version 100

#ifdef GL_ES
precision mediump float;
#endif

// Attributes
attribute vec4 a_position;
attribute vec2 a_texCoord0;
attribute vec4 a_color;

// Uniforms
uniform mat4 u_modelViewProjectionMatrix;

// Varyings
varying vec2 v_texCoord0;
varying vec4 v_frontColor;

void main ()
{
  gl_Position = u_modelViewProjectionMatrix * a_position;
  v_texCoord0 = a_texCoord0;
  v_frontColor = a_color;
}
