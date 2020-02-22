#version 100

precision mediump float;
precision mediump int;

uniform sampler2D u_texUnit;
uniform int u_textureId;

varying vec4 v_frontColor;
varying vec2 v_texCoord0;

void main()
{
  if (u_textureId != 0)
    gl_FragColor = texture2D(u_texUnit, v_texCoord0) * v_frontColor;
  else
    gl_FragColor = v_frontColor;
}
