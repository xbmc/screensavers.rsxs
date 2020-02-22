#version 130

uniform sampler2D m_samp;
in vec2 v_coord;

void main ()
{
  gl_FragColor = vec4(texture(m_samp, v_coord));
}
