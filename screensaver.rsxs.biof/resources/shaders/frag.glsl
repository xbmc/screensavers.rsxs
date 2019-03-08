#version 130

#ifdef GL_ES
precision mediump float;
#endif

struct LightSource
{
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec3 position;
};

// Uniforms
uniform bool u_fogEnabled;
uniform vec3 u_fogColor;
uniform float u_fogStart;
uniform float u_fogEnd;
uniform LightSource u_light0;
uniform bool u_lightEnabled;

// Varyings
varying vec4 v_frontColor;
varying float v_eyeDistance;
varying vec3 v_normalInterp;
varying vec3 v_pos;

// Functions
float calcFogLinear(float distanceToEye)
{
  float f = (u_fogEnd - distanceToEye) / (u_fogEnd - u_fogStart);
  return clamp(f, 0.0, 1.0);
}

void main()
{
  vec4 color;
  color = v_frontColor;

  if (u_lightEnabled)
  {
    vec3 normal = mix(normalize(v_normalInterp), normalize(cross(dFdx(v_pos), dFdy(v_pos))), 1.0);
    vec3 lightDir = normalize(u_light0.position - v_pos);

    float lambertian = max(dot(lightDir,normal), 0.0);
    vec4 specular = vec4(0.0);

    color = clamp(u_light0.ambient * color +
                  lambertian * u_light0.diffuse * color, 0.0, 1.0);
  }

  if (u_fogEnabled)
  {
    float fogFactor = calcFogLinear(v_eyeDistance);
    color.rgb = mix(u_fogColor, color.rgb, fogFactor);
  }

  gl_FragColor = color;
}
