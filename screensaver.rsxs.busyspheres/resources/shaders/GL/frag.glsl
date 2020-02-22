#version 130

// Uniforms
uniform sampler2D u_texUnit;

// Varyings
in vec4 v_frontColor;
in vec2 v_texCoord0;

out vec4 fragColor;

void main()
{
  fragColor = texture(u_texUnit, v_texCoord0) * v_frontColor;
}
