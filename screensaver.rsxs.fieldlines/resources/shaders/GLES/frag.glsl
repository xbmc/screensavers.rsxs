#version 100

precision mediump float;

// Varyings
varying vec4 v_col;

void main()
{
  gl_FragColor = v_col;
}
