#version 130

// Attributes
in vec4 a_position;
in vec4 a_color;

// Uniforms
uniform mat4 u_modelViewProjectionMatrix;

// Varyings
out vec4 v_frontColor;

void main ()
{
  gl_Position = u_modelViewProjectionMatrix * a_position;

  v_frontColor = a_color;
}
