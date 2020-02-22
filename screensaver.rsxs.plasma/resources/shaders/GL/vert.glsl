#version 130

in vec4 m_attrpos;
in vec2 m_attrcord;

out vec2 v_coord;

void main ()
{
  gl_Position = m_attrpos;
  v_coord = m_attrcord;
}
