#version 100

#ifdef GL_ES
precision mediump float;
#endif

// Uniforms
uniform sampler2D u_texUnit;

// Varyings
varying vec4 v_frontColor;
varying vec2 v_texCoord0;

void main()
{
  gl_FragColor = texture2D(u_texUnit, v_texCoord0) * v_frontColor;
}
