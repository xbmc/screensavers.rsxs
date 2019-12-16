#version 100

#extension GL_OES_standard_derivatives : enable

precision mediump float;

struct LightSource
{
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 position;
};

// Uniforms
uniform bool u_fogEnabled;
uniform vec3 u_fogColor;
uniform float u_fogStart;
uniform float u_fogEnd;
uniform bool u_lightEnabled;
uniform LightSource u_light0;

// Varyings
varying vec4 v_frontColor;
varying float v_eyeDistance;
varying vec4 v_ambientAndEmission;
varying vec3 v_normal;
varying vec3 v_light0Vector;
varying vec3 v_light0HalfVector;

// Functions
vec4 calcPerFragmentLighting()
{
  vec4 color = v_ambientAndEmission;
  vec3 normal = normalize(v_normal);

  float cosL = max(0.0, dot(normal, normalize(v_light0Vector)));
  float cosH = dot(normal, normalize(v_light0HalfVector));

  color += (u_light0.diffuse * vec4(0.8, 0.8, 0.8, 1.0)) * cosL;
  if (cosH > 0.0)
    color += u_light0.specular * pow(cosH, 1.0);

  color.a = 1.0;

  return clamp(color, 0.0, 1.0);
}

float calcFogLinear(float distanceToEye)
{
  float f = (u_fogEnd - distanceToEye) / (u_fogEnd - u_fogStart);
  return clamp(f, 0.0, 1.0);
}

void main()
{
  vec4 color = v_frontColor;

  if (u_lightEnabled)
    color *= calcPerFragmentLighting();

  if (u_fogEnabled)
  {
    float fogFactor = calcFogLinear(v_eyeDistance);
    color.rgb = mix(u_fogColor, color.rgb, fogFactor);
  }

  gl_FragColor = color;
}
