#version 100

precision mediump float;
precision mediump int;

// Structs
struct Fog
{
  int use;
  float start;
  float end;
};

// Attributes
attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec3 a_color;
attribute vec2 a_coord;

// Uniforms
uniform Fog u_fog;
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_transposeAdjointModelViewMatrix;
uniform int u_useSphere;

// Varyings
varying vec2 v_texcoord0;
varying float v_fogAmount;
varying vec3 v_color;
varying vec3 v_pos;
varying vec3 v_normalInterp;

float fogFactorLinear(const float dist, const float start, const float end)
{
  return 1.0 - clamp((end - dist) / (end - start), 0.0, 1.0);
}

vec2 SphereMap(const vec3 U, const vec3 N)
{
  vec3 R;

  R = reflect( U, N );
  R.z += 1.0;               // half-angle
  R = normalize( R );
  return R.xy * 0.5 + 0.5;  // [-1.0, 1.0] --> [0.0, 1,0]
}

void main()
{
  vec4 vertex = u_modelViewMatrix * vec4(a_position, 1.0);

  v_normalInterp = u_transposeAdjointModelViewMatrix * a_normal;
  v_pos = vec3(vertex) / vertex.w;
  v_color = a_color;

  gl_Position = u_projectionMatrix * vertex;

  // Calculate Fog amount if used
  if (u_fog.use == 1)
  {
    float fogDistance = length(gl_Position.xyz);
    v_fogAmount = fogFactorLinear(fogDistance, u_fog.start, u_fog.end);
  }

  if (u_useSphere == 0)
  {
    v_texcoord0 = a_coord;
  }
  else
  {
    // gen texture coordinates
    v_texcoord0 = SphereMap(normalize(v_pos), normalize(v_normalInterp));

    // flip for translation from Jitter to GL coordinate space
//     vec2 texdim0 = vec2(abs(gl_TextureMatrix[0][0][0]),abs(gl_TextureMatrix[0][1][1]));
//     v_texcoord0 = vec2(sMap.x, texdim0.y - sMap.y);
  }
}
