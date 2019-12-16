#version 130

// Uniforms
uniform sampler2D u_texUnit;
uniform int u_colorUsed;
uniform vec4 u_color;
uniform int u_textureUsed;

// Varyings
in vec2 v_texCoord0;
in vec4 v_frontColor;

void main()
{
  vec4 color;
  if (u_colorUsed == 0)
    color = v_frontColor;
  else
    color = u_color;

  if (u_textureUsed != 0)
    gl_FragColor = texture2D(u_texUnit, v_texCoord0) * color;
  else
    gl_FragColor = color;
}
