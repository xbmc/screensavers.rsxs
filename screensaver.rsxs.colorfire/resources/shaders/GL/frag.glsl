#version 130

// Uniforms
uniform sampler2D u_texUnit;
uniform int u_type;

// Varyings
in vec4 v_frontColor;
in vec2 v_texCoord0;

out vec4 fragColor;

void main()
{
  if (u_type != 0)
    fragColor = texture(u_texUnit, v_texCoord0) * v_frontColor;
  else
    fragColor = texture(u_texUnit, v_texCoord0).rrra * v_frontColor;
}
