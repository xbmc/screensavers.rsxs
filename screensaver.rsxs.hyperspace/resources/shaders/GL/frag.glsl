#version 130

#define SHADER_NORMAL 0
#define SHADER_GOO 1
#define SHADER_TUNNEL 2

#define USE_FASTEST 0
#define USE_NICEST 1

#define FOG_HINT USE_FASTEST

// Uniforms
uniform sampler2D u_texUnit0;
uniform sampler2D u_texUnit1;
uniform sampler2D u_texUnit2;
uniform samplerCube u_normaltex0;
uniform samplerCube u_normaltex1;
uniform samplerCube u_tex;
uniform int u_textureUsed;
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

// Functions
float calcFogLinear(float distanceToEye)
{
  float f = (u_fogEnd - distanceToEye) / (u_fogEnd - u_fogStart);
  return clamp(f, 0.0, 1.0);
}

void main()
{
#if FOG_HINT == USE_FASTEST
  float fogFactor = v_fogFactor;
#elif FOG_HINT == USE_NICEST
  float fogFactor = calcFogLinear(v_eyeDistance);
#endif

  if (u_activeShader == SHADER_TUNNEL)
  {
    vec4 texcol0 = texture2D(u_texUnit0, v_texCoord0.xy);
    texcol0 += 0.5 * texture2D(u_texUnit0, v_texCoord0.xy * vec2(3.0, 2.0));
    vec4 texcol1 = texture2D(u_texUnit1, v_texCoord0.xy);
    texcol1 += 0.5 * texture2D(u_texUnit1, v_texCoord0.xy * vec2(3.0, 2.0));
    vec4 color = mix(texcol0, texcol1, v_frontColor.a);

    // Note that max value of color at this point is 1.5
    // Overbrighten and modulate with vertex color using a mix of regular multiplicative and subtractive alpha
    color.rgb = 2.0 * mix(v_frontColor.rgb * color.rgb,
                          max(vec3(0.0, 0.0, 0.0),
                          (v_frontColor.rgb * vec3(1.5, 1.5, 1.5)) + color.rgb - vec3(1.5, 1.5, 1.5)), 0.5);
    // Depth cue
    gl_FragColor = mix(u_fogColor, color, clamp((u_fogEnd + v_position.z) * fogFactor, 0.0, 1.0));
  }
  else if (u_activeShader == SHADER_GOO)
  {
    vec4 normal0 = textureCube(u_normaltex0, v_texCoord0.xyz) * 2.0 - 1.0;
    vec4 normal1 = textureCube(u_normaltex1, v_texCoord0.xyz) * 2.0 - 1.0;
    vec4 texnormal = mix(normal0, normal1, v_frontColor.a);
    // Reflection vector
    vec3 eyevec = normalize(v_position.xyz);
    float eyedotnorm = dot(eyevec, texnormal.xyz);
    vec3 reflectvec = (texnormal.xyz * (eyedotnorm * 2.0)) - eyevec;
    // Reflected color
    vec4 color = textureCube(u_tex, reflectvec);
    // Fresnel reflection
    float fresnelalpha = 1.0 - abs(eyedotnorm);
    color *= fresnelalpha * fresnelalpha * fresnelalpha;
    // Overbrighten and modulate with vertex color using a mix of regular multiplicative and subtractive alpha
    color.rgb = 2.0 * mix(color.rgb * v_frontColor.rgb, max(vec3(0.0, 0.0, 0.0), color.rgb + v_frontColor.rgb - vec3(1.0, 1.0, 1.0)), 0.6);
    // Depth cue
    gl_FragColor = mix(u_fogColor, color, clamp((u_fogEnd + v_position.z) * fogFactor, 0.0, 1.0));
  }
  else
  {
    vec4 color = v_frontColor;

    if (u_fogEnabled != 0)
      color.rgb = mix(u_fogColor.rgb, color.rgb, fogFactor);

    if (u_textureUsed != 0)
      gl_FragColor = texture2D(u_texUnit0, v_texCoord0.xy) * color;
    else
      gl_FragColor = color;
  }
}
