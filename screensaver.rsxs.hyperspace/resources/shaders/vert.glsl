#version 130

#ifdef GL_ES
precision mediump float;
#endif

#define SHADER_NORMAL 0
#define SHADER_GOO 1
#define SHADER_TUNNEL 2

#define USE_FASTEST 0
#define USE_NICEST 1

#define FOG_HINT USE_FASTEST

// Attributes
attribute vec4 a_position;
attribute vec4 a_normal;
attribute vec4 a_color;
attribute vec2 a_coord;

// Uniforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform vec4 u_uniformColor;
uniform int u_uniformColorUsed;
uniform int u_fogEnabled;
uniform vec4 u_fogColor;
uniform float u_fogStart;
uniform float u_fogEnd;
uniform int u_activeShader;

// Varyings
varying vec4 v_position;
varying vec4 v_texCoord0;
varying vec4 v_frontColor;
#if FOG_HINT == USE_FASTEST
varying float v_fogFactor;
#elif FOG_HINT == USE_NICEST
varying float v_eyeDistance;
#endif

// Shader variables
vec3 normal;
vec4 vertexPositionInEye;

// Functions
float calcFogLinear(float distanceToEye)
{
  float f = (u_fogEnd - distanceToEye) / (u_fogEnd - u_fogStart);
  return clamp(f, 0.0, 1.0);
}

void main ()
{
  if (u_fogEnabled != 0)
  {
    vertexPositionInEye = u_modelViewMatrix * a_position;
  #if FOG_HINT == USE_FASTEST
    v_fogFactor = calcFogLinear(-vertexPositionInEye.z);
  #elif FOG_HINT == USE_NICEST
    v_eyeDistance = -vertexPositionInEye.z;
  #endif

  }

  gl_Position = u_projectionMatrix * u_modelViewMatrix * a_position;

  v_position = u_modelViewMatrix * a_position;
  if (u_activeShader == SHADER_GOO)
    v_texCoord0 = a_normal;
  else
    v_texCoord0 = vec4(a_coord, 1.0, 0.0);
  if (u_uniformColorUsed != 0)
    v_frontColor = u_uniformColor;
  else
    v_frontColor = a_color;
}
