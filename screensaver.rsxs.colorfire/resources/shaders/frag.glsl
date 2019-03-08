#version 100

precision mediump float;

// Uniforms
uniform sampler2D u_texUnit;
uniform int u_type;

// Varyings
varying vec4 v_frontColor;
varying vec2 v_texCoord0;

void main()
{
  if (u_type != 0)
    gl_FragColor = texture2D(u_texUnit, v_texCoord0) * v_frontColor;
  else
    gl_FragColor = texture2D(u_texUnit, v_texCoord0).rrra * v_frontColor;
}
