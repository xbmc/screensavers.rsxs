#version 130

// Attributes
in vec4 a_position;
in vec4 a_color;
in vec2 a_coord;

// Uniforms
uniform mat4 u_projModelMatrix;

// Varyings
out vec4 v_frontColor;
out vec2 v_texCoord0;

void main()
{
  gl_Position = u_projModelMatrix * a_position;

  v_texCoord0 = a_coord;
  v_frontColor = a_color;
  v_frontColor.g *= 1.15; /* Give green a bit of a boost */
}
