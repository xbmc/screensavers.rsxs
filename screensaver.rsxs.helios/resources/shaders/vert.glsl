#version 130

precision mediump float;

// Attributes
attribute vec4 a_position;
attribute vec4 a_normal;
attribute vec4 a_color;
attribute vec2 a_coord;

// Uniforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_transposeAdjointModelViewMatrix;
uniform int u_type;

// Varyings
varying vec4 v_frontColor;
varying vec2 v_texCoord0;

vec2 SphereMap(const vec3 U, const vec3 N)
{
  vec3 R;

  R = reflect( U, N );
  R.z += 1.0;              // half-angle
  R = normalize( R );
  return R.xy * 0.5 + 0.5; // [-1.0, 1.0] --> [0.0, 1,0]
}

void main ()
{
  // eye coordinates
  vec4 position = u_modelViewMatrix * a_position;

  //  set vertex position in clip space
  gl_Position = u_projectionMatrix * position;

  if (u_type == 2)
  {
    v_texCoord0 = a_coord;
  }
  else if (u_type == 3)
  {
    vec3 p = (vec3(position))/position.w;

    // transformed normal
    vec3 n = u_transposeAdjointModelViewMatrix * a_normal.rgb;

    // gen texture coordinates
    vec2 sMap = SphereMap(normalize(p), normalize(n));

    // flip for translation from Jitter to GL coordinate space
    vec2 texdim0 = vec2(abs(gl_TextureMatrix[0][0][0]),abs(gl_TextureMatrix[0][1][1]));
    v_texCoord0 = vec2(sMap.x, texdim0.y - sMap.y);
  }

  v_frontColor = a_color;
}
