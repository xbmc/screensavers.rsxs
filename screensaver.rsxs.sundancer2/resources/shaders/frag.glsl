#ifdef GL_ES
precision mediump float;
#endif

varying vec4 v_frontColor;

void main()
{
  gl_FragColor = v_frontColor;
}
