#ifdef GL_ES
precision mediump float;
#endif

#define TEXTURE_RGB 1
#define TEXTURE_RGBA 2
#define TEXTURE_ALPHA 3

// Structs
struct LightSource
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 position;
  float shininess;
};

struct Fog
{
  int use;
  float start;
  float end;
};

// Uniforms
uniform int u_textureId;
uniform int u_textureStyle;
uniform int u_mixTexture;
uniform float u_useFlat;
uniform int u_useLighting;
uniform LightSource u_light;
uniform Fog u_fog;
uniform sampler2D u_texUnit;

// Varyings
varying vec2 v_texcoord0;
varying float v_fogAmount;
varying vec3 v_color;
varying vec3 v_pos;
varying vec3 v_normalInterp;

void main()
{
  float alpha = 1.0;
  vec3 usedColor = v_color;
  if (u_textureId != 0)
  {
    if (u_textureStyle == TEXTURE_RGB)
    {
      usedColor = texture2D(u_texUnit, v_texcoord0).rgb;
    }
    else if (u_textureStyle == TEXTURE_RGBA)
    {
      vec4 texColor = texture2D(u_texUnit, v_texcoord0);
      if (u_mixTexture != 0)
        usedColor = mix(texColor.rgb, usedColor, 0.6);
      else if (texColor.a > 0.1)
        usedColor = texColor.rgb;
    }
    else if (u_textureStyle == TEXTURE_ALPHA)
    {
      alpha = texture2D(u_texUnit, v_texcoord0).a;
    }
  }

  if (u_useLighting == 1)
  {
    vec3 normal = mix(normalize(v_normalInterp), normalize(cross(dFdx(v_pos), dFdy(v_pos))), u_useFlat);
    vec3 lightDir = normalize(u_light.position - v_pos);

    float lambertian = max(dot(lightDir,normal), 0.0);
    vec3 specular = vec3(0.0);

    if (u_light.shininess > 0.0 && lambertian > 0.0)
    {
      vec3 viewDir = normalize(-v_pos);
      vec3 halfDir = normalize(lightDir + viewDir);
      float specAngle = max(dot(halfDir, normal), 0.0);
      specular = pow(specAngle, u_light.shininess) * u_light.specular;
    }

    usedColor = clamp(u_light.ambient * usedColor +
                  lambertian * u_light.diffuse * usedColor +
                  specular, 0.0, 1.0);
  }

  // Mix Fog color with other if used
  if (u_fog.use == 1)
  {
    const vec3 fogColor = vec3(0.0); // Black
    usedColor = mix(usedColor, fogColor, v_fogAmount);
  }

  gl_FragColor = vec4(usedColor, alpha);
}
