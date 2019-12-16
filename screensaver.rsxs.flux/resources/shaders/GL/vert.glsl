#version 130

// Structs
struct Light {
  // all lights
  vec4 position; // if directional light, this must be normalized (so that we dont have to normalize it here)
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  // point light & spotlight
  float constantAttenuation;
  float linearAttenuation;
  float quadraticAttenuation;

  // spotlight
  vec3 spotDirection;
  float spotExponent;
  float spotCutoffAngleCos;
};

struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 emission;
  float shininess;
};

// Attributes
in vec3 a_normal;
in vec4 a_position;
in vec2 a_coord;

// Uniforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_modelViewProjectionMatrix;
uniform mat3 u_transposeAdjointModelViewMatrix;
uniform Light u_light0;
uniform Material u_material;

// Varyings
out vec4 v_ambientAndEmission;
out vec3 v_normal;
out vec4 v_frontColor;
out vec3 v_light0Vector;
out vec3 v_light0HalfVector;
out vec2 v_texCoord0;

// Shader variables
vec4 vertexPositionInEye;

void calcLightVaryingsForFragmentShader(Light light, vec3 eyeVector, out vec3 lightVector, out vec3 halfVector)
{
  v_ambientAndEmission += light.ambient * u_material.ambient;
  if (light.position.w != 0.0)
      lightVector = light.position.xyz - vertexPositionInEye.xyz;
  else
      lightVector = light.position.xyz;
  halfVector = normalize(eyeVector + lightVector);
}

#define LIGHT_MODEL_LOCAL_VIEWER_ENABLED 1
void calcLightingVaryingsForFragmentShader()
{
  vec3 eyeVector;
  #if LIGHT_MODEL_LOCAL_VIEWER_ENABLED == 1
  eyeVector = normalize(-vertexPositionInEye.xyz);
  #elif LIGHT_MODEL_LOCAL_VIEWER_ENABLED == 0
  eyeVector = vec3(0.0, 0.0, 1.0);
  #endif

  v_ambientAndEmission = u_material.ambient;
  v_ambientAndEmission += u_material.emission;

  calcLightVaryingsForFragmentShader(u_light0, eyeVector, v_light0Vector, v_light0HalfVector);
}

void main ()
{
  gl_Position = u_modelViewProjectionMatrix * a_position;
  v_normal = u_transposeAdjointModelViewMatrix * a_normal;
  v_texCoord0 = a_coord;
  vertexPositionInEye = u_modelViewMatrix * a_position;

  calcLightingVaryingsForFragmentShader();
}
