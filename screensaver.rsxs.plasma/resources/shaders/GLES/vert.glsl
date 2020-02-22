#version 100

precision mediump float;

attribute vec4 m_attrpos;
attribute vec2 m_attrcord;

varying vec2 v_coord;

void main ()
{
  gl_Position = m_attrpos;
  v_coord = m_attrcord;
}
