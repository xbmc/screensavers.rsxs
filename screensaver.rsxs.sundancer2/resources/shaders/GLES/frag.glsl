#version 100

precision mediump float;

varying vec4 v_frontColor;

void main()
{
  gl_FragColor = v_frontColor;
}
