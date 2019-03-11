#version 100

#ifdef GL_ES
precision mediump float;
#endif

// Varyings
varying vec4 v_frontColor;

void main()
{
  gl_FragColor = v_frontColor;
}
