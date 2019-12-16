#version 130

struct LightSource
{
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 position;
};

// Attributes
in vec3 a_normal;
in vec3 a_position;
in vec4 a_color;

// Uniforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_transposeAdjointModelViewMatrix;
uniform bool u_fogEnabled;
uniform bool u_lightEnabled;
uniform float u_pointSize;
uniform LightSource u_light0;

// Varyings
out vec4 v_frontColor;
out float v_eyeDistance;
out vec4 v_ambientAndEmission;
out vec3 v_normal;
out vec3 v_light0Vector;
out vec3 v_light0HalfVector;

void calcLightingVaryingsForFragmentShader()
{
  bool lightModelLocalViewerEnabled = true;
  vec3 eyeVector = vec3(0.0, 0.0, 1.0);

  v_ambientAndEmission = vec4(0.2, 0.2, 0.2, 1.0) * vec4(0.2, 0.2, 0.2, 1.0);
  v_ambientAndEmission += u_light0.ambient * vec4(0.2, 0.2, 0.2, 1.0);
  v_light0Vector = u_light0.position.xyz;
  v_light0HalfVector = normalize(eyeVector + v_light0Vector);
}

void main()
{
  vec4 vertexPositionInEye = u_modelViewMatrix * vec4(a_position, 1.0);
  gl_Position = u_projectionMatrix * vertexPositionInEye;

  v_normal = u_transposeAdjointModelViewMatrix * a_normal;
  v_frontColor = a_color;

  if (u_lightEnabled)
    calcLightingVaryingsForFragmentShader();

  if (u_fogEnabled)
    v_eyeDistance = -vertexPositionInEye.z;
}
