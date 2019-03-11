#version 130

// Uniforms
uniform sampler2D u_texture0Sampler;
uniform int u_geometry;

// Varyings
smooth in vec4 v_frontColor;
in vec2 v_coord;
in vec4 v_position;

void main()
{
  if (u_geometry == 0 || u_geometry == 1)
  {
    gl_FragColor = v_frontColor;
    gl_FragColor.rgb *= texture2D(u_texture0Sampler, v_coord).rrr;
  }
  else if (u_geometry == 2)
  {
    gl_FragColor = v_frontColor;
  }
}
