#version 130

uniform sampler2D u_texUnit;

in vec4 v_frontColor;
in vec2 v_texCoord0;

void main()
{
  vec4 color = v_frontColor;
  /* Use GL_DECAL so texels aren't multipied by the colors on the screen, e.g
   * the black set by calls to glClear(GL_COLOR_BUFFER_BIT) */
  vec4 texture0Color = texture2D(u_texUnit, v_texCoord0);
  color.rgb = mix(color.rgb, texture0Color.rgb, texture0Color.a);
  gl_FragColor = color;
}
