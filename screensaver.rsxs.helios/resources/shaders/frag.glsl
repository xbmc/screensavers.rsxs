#version 130

precision mediump float;

// Uniforms
uniform int u_type;
uniform sampler2D u_texUnit;

// Varyings
varying vec4 v_frontColor;
varying vec2 v_texCoord0;

void main()
{
  if (u_type == 2 || u_type == 3)
  {
    gl_FragColor = texture2D(u_texUnit, v_texCoord0) * v_frontColor;
  }
  else
  {
    gl_FragColor = v_frontColor;
  }
}
