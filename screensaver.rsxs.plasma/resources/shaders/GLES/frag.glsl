#version 100

precision mediump float;

uniform sampler2D m_samp;
varying vec2 v_coord;

void main ()
{
  gl_FragColor = vec4(texture2D(m_samp, v_coord));
}
