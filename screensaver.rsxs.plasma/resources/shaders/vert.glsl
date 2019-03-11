#ifdef GL_ES
precision mediump float;
#endif

attribute vec4 m_attrpos;
attribute vec2 m_attrcord;

varying vec2 v_coord;

uniform mat4 m_proj;
uniform mat4 m_model;

void main ()
{
  mat4 mvp = m_proj * m_model;
  gl_Position = mvp * m_attrpos;
  v_coord = m_attrcord;
}
