#version 130

#ifdef GL_ES
precision mediump float;
#endif

// Attributes
attribute vec4 a_position;
attribute vec4 a_color;
attribute vec3 a_normal;

// Uniforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_transposeAdjointModelViewMatrix;
uniform bool u_fogEnabled;

// Varyings
varying vec4 v_frontColor;
varying vec3 v_normalInterp;
varying float v_eyeDistance;
varying vec3 v_pos;

void main()
{
  gl_Position = u_projectionMatrix * u_modelViewMatrix  * a_position;

  vec4 vertex = u_modelViewMatrix * a_position;
  vec4 vertexPositionInEye = u_modelViewMatrix * a_position;

  v_pos = vec3(vertex) / vertex.w;
  v_normalInterp = normalize(u_transposeAdjointModelViewMatrix * a_normal);
  v_frontColor = a_color;
  if (u_fogEnabled)
    v_eyeDistance = -vertexPositionInEye.z;
}
