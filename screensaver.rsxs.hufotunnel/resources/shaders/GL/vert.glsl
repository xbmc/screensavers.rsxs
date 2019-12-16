#version 130

// Attributes
in vec4 a_position;
in vec2 a_texCoord0;
in vec4 a_color;

// Uniforms
uniform mat4 u_modelViewProjectionMatrix;

// Varyings
out vec2 v_texCoord0;
out vec4 v_frontColor;

void main ()
{
  gl_Position = u_modelViewProjectionMatrix * a_position;
  v_texCoord0 = a_texCoord0;
  v_frontColor = a_color;
}
