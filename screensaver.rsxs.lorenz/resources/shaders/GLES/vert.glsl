#version 100

struct Light {
  // all lights
  vec4 position;
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
attribute vec4 a_position;
attribute vec4 a_color;
attribute vec3 a_normal;

// Uniforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_transposeAdjointModelViewMatrix;
uniform vec4 u_globalAmbientColor;
uniform bool u_lightingEnabled;
uniform Light u_light0;
uniform Material u_material;

// Varyings
varying vec4 v_frontColor;

// Shader variables
vec3 normal;
vec4 vertexPositionInEye;

float calcBasicAttenuationFactor(Light light, vec3 lightVector)
{
  vec3 attenuationDistance;
  attenuationDistance.x = 1.0;
  attenuationDistance.z = dot(lightVector, lightVector);
  attenuationDistance.y = sqrt(attenuationDistance.z);
  return 1.0 / dot(attenuationDistance, vec3(light.constantAttenuation, light.linearAttenuation, light.quadraticAttenuation));
}

vec4 calcSpotLightWithLocalViewer(Light light)
{
  vec3 lightVector = light.position.xyz - vertexPositionInEye.xyz;
  float attenuationFactor = calcBasicAttenuationFactor(light, lightVector);

  lightVector = normalize(lightVector);

  vec3 eyeVector = normalize(-vertexPositionInEye.xyz);

  vec3 halfVector = normalize(eyeVector + lightVector);
  float cosL = max(0.0, dot(normal, lightVector));
  float cosH = dot(normal, halfVector);

  vec4 color = (light.ambient * u_material.ambient);
  color += (light.diffuse * u_material.diffuse) * cosL;
  if (cosH > 0.0)
    color += (u_material.specular * light.specular) * pow(cosH, u_material.shininess);

  color *= attenuationFactor;

  return color;
}

vec4 calcPointLightWithLocalViewer(Light light)
{
  vec3 lightVector = light.position.xyz - vertexPositionInEye.xyz;
  float attenuationFactor = calcBasicAttenuationFactor(light, lightVector);

  lightVector = normalize(lightVector);

  vec3 eyeVector = normalize(-vertexPositionInEye.xyz);

  vec3 halfVector = normalize(eyeVector + lightVector);
  float cosL = max(0.0, dot(normal, lightVector));
  float cosH = dot(normal, halfVector);

  vec4 color = (light.ambient * u_material.ambient);
  color += (light.diffuse * u_material.diffuse) * cosL;
  if (cosH > 0.0)
    color += (u_material.specular * light.specular) * pow(cosH, u_material.shininess);

  color *= attenuationFactor;

  return color;
}

vec4 calcDirectionalLightWithLocalViewer(Light light)
{
  vec3 lightVector = light.position.xyz;
  vec3 eyeVector = normalize(-vertexPositionInEye.xyz);

  vec3 halfVector = normalize(eyeVector + lightVector);
  float cosL = max(0.0, dot(normal, lightVector));
  float cosH = dot(normal, halfVector);

  vec4 color = (light.ambient * u_material.ambient);
  color += (light.diffuse * u_material.diffuse) * cosL;
  if (cosH > 0.0)
    color += (u_material.specular * light.specular) * pow(cosH, u_material.shininess);

  return color;
}

vec4 calcLight(Light light)
{
  vec4 color;
  if (light.position.w != 0.0)
  {
    if (light.spotCutoffAngleCos > -1.0)
      color = calcSpotLightWithLocalViewer(light);
    else
      color = calcPointLightWithLocalViewer(light);
  }
  else
  {
    color = calcDirectionalLightWithLocalViewer(light);
  }

  return color;
}

vec4 calcPerVertexLighting()
{
  vec4 color = u_material.ambient * u_globalAmbientColor;
  color += u_material.emission;
  color += calcLight(u_light0);
  color.a = u_material.diffuse.a;

  return clamp(color, 0.0, 1.0);
}

void main ()
{
  gl_Position = u_projectionMatrix * u_modelViewMatrix * a_position;

  normal = normalize(u_transposeAdjointModelViewMatrix * a_normal);
  vertexPositionInEye = u_modelViewMatrix * a_position;

  if (u_lightingEnabled)
    v_frontColor = calcPerVertexLighting();
  else
    v_frontColor = mix(calcPerVertexLighting(), a_color, 0.4);
}
