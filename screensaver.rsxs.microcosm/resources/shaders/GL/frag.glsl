#version 130

struct LightSource
{
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 position;
};

// Uniforms
uniform sampler1D u_diffusetex;
uniform LightSource u_light0;
uniform LightSource u_light1;
uniform bool u_fogEnabled;
uniform vec4 u_fogColor;
uniform float u_fogStart;
uniform float u_fogEnd;

// Varyings
varying vec4 vsPosition;
varying vec3 vsNormal;
varying vec3 vsLight0Vec;
varying vec3 vsLight1Vec;
varying vec3 vsEyeVec;
varying vec4 vsTexCoord;

// values
vec4 diffuse;
vec3 specular;

// Should just have one light function and pass the light index as a variable.  Using a constant
// index to gl_LightSource works around a problem in legacy ATI drivers.
void directionalLight0(in vec3 lightvec, in vec3 normal, in vec3 eyevec)
{
  // diffuse
  float norm_dot_dir = max(0.0, dot(normal, lightvec));
  diffuse.rgb += max(u_light0.diffuse.rgb * norm_dot_dir, u_light0.ambient.rgb);
  if(norm_dot_dir > 0.0)
  {
    // blinn specular
    //vec3 halfvec = normalize(eyevec + lightvec);
    //specular += gl_LightSource[index].specular.rgb * pow(max(dot(normal, halfvec), 0.0), 50.0);
    // phong specular
    //vec3 reflectvec = (normal * (2.0 * norm_dot_dir)) - lightvec;
    vec3 reflectvec = reflect(-lightvec, normal);
    specular += u_light0.specular.rgb * pow(max(dot(eyevec, reflectvec), 0.0), 50.0);
  }
}
void directionalLight1(in vec3 lightvec, in vec3 normal, in vec3 eyevec)
{
  float norm_dot_dir = max(0.0, dot(normal, lightvec));
  diffuse.rgb += max(u_light1.diffuse.rgb * norm_dot_dir, u_light1.ambient.rgb);
  if(norm_dot_dir > 0.0)
  {
    vec3 reflectvec = reflect(-lightvec, normal);
    specular += u_light1.specular.rgb * pow(max(dot(eyevec, reflectvec), 0.0), 50.0);
  }
}
void main(void)
{
  diffuse = vec4(0.0, 0.0, 0.0, 1.0);
  specular = vec3(0.0, 0.0, 0.0);
  vec3 vsNormalNormalized = normalize(vsNormal);
  vec3 vsEyeVecNormalized = normalize(vsEyeVec);
  directionalLight0(normalize(vsLight0Vec), vsNormalNormalized, vsEyeVecNormalized);
  directionalLight1(normalize(vsLight1Vec), vsNormalNormalized, vsEyeVecNormalized);
  vec4 texcol0 = texture1D(u_diffusetex, vsTexCoord.x);
  vec4 texcol1 = texture1D(u_diffusetex, vsTexCoord.y);
  diffuse *= mix(texcol0, texcol1, vsTexCoord.z);
  // pre-multiplied alpha
  diffuse.rgb *= diffuse.a;
  // smart specular addition
  diffuse.rgb += specular * (vec3(1.0, 1.0, 1.0) - diffuse.rgb);
  if (u_fogEnabled)
    diffuse = mix(u_fogColor, diffuse, clamp((u_fogEnd - length(vsPosition.xyz)) /** gl_Fog.scale*/, 0.0, 1.0));
  gl_FragColor = diffuse;
}
