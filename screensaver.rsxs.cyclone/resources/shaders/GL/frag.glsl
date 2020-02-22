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

// Uniforms
uniform int u_lighting;
uniform Light u_light0;
uniform Material u_material;
uniform vec4 u_uniformColor;

// Varyings
in vec4 v_frontColor;
in vec3 v_normal;
in vec4 v_ambientAndEmission;
in vec3 v_light0Vector;
in vec3 v_light0HalfVector;

out vec4 fragColor;

float calcSpotFactor(Light light, vec3 lightVector)
{
  float spotFactor = dot(normalize(-lightVector), normalize(light.spotDirection));
  if (spotFactor >= light.spotCutoffAngleCos)
    return pow(spotFactor, light.spotExponent);
  else
    return 0.0;
}

float calcBasicAttenuationFactor(Light light, vec3 lightVector)
{
  vec3 attenuationDistance;
  attenuationDistance.x = 1.0;
  attenuationDistance.z = dot(lightVector, lightVector);
  attenuationDistance.y = sqrt(attenuationDistance.z);
  return 1.0 / dot(attenuationDistance, vec3(light.constantAttenuation, light.linearAttenuation, light.quadraticAttenuation));
}

vec4 calcSpotLight(Light light, Material material, vec3 normal, vec3 lightVector, vec3 halfVector)
{
  // if clause here because of heavy computation?
  float attenuationFactor = calcBasicAttenuationFactor(light, lightVector);
  attenuationFactor *= calcSpotFactor(light, lightVector);

  float cosL = max(0.0, dot(normal, normalize(lightVector)));
  float cosH = dot(normal, normalize(halfVector));

  vec4 color = (light.diffuse * u_material.diffuse) * cosL;
  if (cosH > 0.0) {
    color += (u_material.specular * light.specular) * pow(cosH, u_material.shininess);
  }

  color *= attenuationFactor;

  return color;
}

vec4 calcDirectionalLight(Light light, Material material, vec3 normal, vec3 lightVector, vec3 halfVector)
{
  float cosL = max(0.0, dot(normal, normalize(lightVector)));
  float cosH = dot(normal, normalize(halfVector));

  vec4 color = (light.diffuse * u_material.diffuse) * cosL;
  if (cosH > 0.0)
    color += (u_material.specular * light.specular) * pow(cosH, u_material.shininess);

  return color;
}

vec4 calcPointLight(Light light, Material material, vec3 normal, vec3 lightVector, vec3 halfVector)
{
  // if clause here because of heavy computation?
  float attenuationFactor = calcBasicAttenuationFactor(light, lightVector);

  float cosL = max(0.0, dot(normal, normalize(lightVector)));
  float cosH = dot(normal, normalize(halfVector));

  vec4 color = (light.diffuse * u_material.diffuse) * cosL;
  if (cosH > 0.0)
    color += (u_material.specular * light.specular) * pow(cosH, u_material.shininess);

  color *= attenuationFactor;

  return color;
}

vec4 calcLight(Light light, Material material, vec3 normal, vec3 lightVector, vec3 halfVector)
{
  vec4 color;
  if (light.position.w != 0.0)
  {
    if (light.spotCutoffAngleCos > -1.0)
      color = calcSpotLight(light, material, normal, lightVector, halfVector);
    else
      color = calcPointLight(light, material, normal, lightVector, halfVector);
  }
  else
    color = calcDirectionalLight(light, material, normal, lightVector, halfVector);

  return color;
}

vec4 calcPerFragmentLighting()
{
  vec4 color = v_ambientAndEmission;
  vec3 normal = normalize(v_normal);

  color += calcLight(u_light0, u_material, normal, v_light0Vector, v_light0HalfVector);
  color.a = u_material.diffuse.a;

  return clamp(color, 0.0, 1.0);
}

void main()
{
  if (u_lighting == 1)
    fragColor = calcPerFragmentLighting() * u_uniformColor;
  else
    fragColor = v_frontColor;
}
