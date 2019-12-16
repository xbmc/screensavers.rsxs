#version 100

precision mediump float;

struct Light {
  vec4 position;
  vec4 ambient;
  vec4 diffuse;
};

// Attributes
attribute vec4 a_position;
attribute vec4 a_color;

// Uniforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform Light u_light0;

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
  return 1.0 / dot(attenuationDistance, vec3(1.0, 0.0, 0.0));
}

vec4 calcPointLightWithLocalViewer(Light light)
{
  vec3 lightVector = light.position.xyz - vertexPositionInEye.xyz;
  float attenuationFactor = calcBasicAttenuationFactor(light, lightVector);

  lightVector = normalize(lightVector);

  vec3 eyeVector = normalize(-vertexPositionInEye.xyz);

  vec3 halfVector = normalize(eyeVector + lightVector);
  float cosL = max(0.0, dot(normal, lightVector));

  vec4 color = (light.ambient * vec4(0.2, 0.2, 0.2, 1.0));
  color += light.diffuse * cosL;
  color *= attenuationFactor;

  return color;
}

vec4 calcDirectionalLightWithLocalViewer(Light light)
{
  vec3 lightVector = light.position.xyz;
  vec3 eyeVector = normalize(-vertexPositionInEye.xyz);

  vec3 halfVector = normalize(eyeVector + lightVector);
  float cosL = max(0.0, dot(normal, lightVector));

  vec4 color = (light.ambient * vec4(0.2, 0.2, 0.2, 1.0));
  color += light.diffuse * cosL;

  return color;
}

vec4 calcLight(Light light)
{
  vec4 color;
  if (light.position.w != 0.0)
  {
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
  vec4 color = vec4(0.2, 0.2, 0.2, 1.0);
  color += calcLight(u_light0);
  color.a = 0.0;

  return clamp(color, 0.0, 1.0);
}

void main ()
{
  gl_Position = u_projectionMatrix * u_modelViewMatrix * a_position;

  normal = vec3(0.0, 0.0, 1.0);
  vertexPositionInEye = u_modelViewMatrix * a_position;

  v_frontColor = mix(calcPerVertexLighting(), a_color, 0.4);
}
