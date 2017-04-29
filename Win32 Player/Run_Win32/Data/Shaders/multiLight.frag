#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture;
uniform sampler2D gEmissiveTexture;
uniform sampler2D gNoiseTexture;
uniform sampler2D gTexSpecular;

uniform vec4 gColor;
uniform vec4 gAmbientLight;
uniform vec4 gFogColor;
uniform vec4 gDissolveColor;

uniform int gLightCount;
uniform vec3 gLightPosition[16];
uniform vec4 gLightColor[16];
uniform vec3 gLightDirection[16];
uniform float gLightDirectionFactor[16];
uniform float gNearDistance[16];
uniform float gFarDistance[16];
uniform float gNearPower[16];
uniform float gFarPower[16];
uniform float gInnerAngle[16];
uniform float gOuterAngle[16];
uniform float gInnerPower[16];
uniform float gOuterPower[16];
uniform float gSpecularIntensity;
uniform float gSpecularPower;

uniform vec3 gCameraPosition;
uniform float gMinFogDistance;
uniform float gMaxFogDistance;
uniform float gTime;
uniform float gDissolveAmount;
uniform int gEffectState;

in vec4 passColor;
in vec2 passUV0;
in vec3 passPosition;
in vec3 passTangent;
in vec3 passBitangent;

out vec4 outColor;

//------------------------------------------------------------------------
struct SurfaceLightColors
{
   vec3 surfaceColor;
   vec3 specularColor;
};

//------------------------------------------------------------------------
vec4 VectorAsColor(vec3 vec)
{
    return vec4((vec + vec3(1.0f)) * vec3(.5f), 1.0f);
}

//------------------------------------------------------------------------
SurfaceLightColors CalculateSurfaceLight(
   vec3 normal,
   vec3 lightPosition,
   vec3 lightDirection,
   vec3 lightColor,
   float lightDirFactor,
   float specPower,
   float specIntensity,
   float nearDistance, float farDistance,
   float nearPower, float farPower,
   float innerAngle, float outerAngle,
   float innerPower, float outerPower )
{
   // Calculate some intermediate values, like the half vector
   vec3 dirToEye = normalize(gCameraPosition - passPosition);
   float distToEye = length(dirToEye);
   vec3 vectorToLight = lightPosition - passPosition;
   float distToLight = length(vectorToLight);

   //LightDirectionFactor = 1 : our vector to light is the back along the lights direction
   //LightDirectionFactor = 0 : it's the actual vector TO the light
   vec3 dirToLight = mix(vectorToLight / distToLight, -lightDirection, lightDirFactor);
   //Distance for "directional lights" is the planar distance, instead of the full vector distance
   distToLight = mix(distToLight, dot(vectorToLight, -lightDirection), lightDirFactor);

   vec3 halfVector = normalize(dirToLight + dirToEye);
   float angle = dot(lightDirection, -dirToLight);

   // Calculate falloff due to cone angle & distance
   float distanceAttenuation = mix(nearPower, farPower, smoothstep(nearDistance, farDistance, distToLight));
   float angleAttenuation = mix(innerPower, outerPower, smoothstep(innerAngle, outerAngle, angle));
   float attenuation = angleAttenuation * distanceAttenuation;

   // Dot3 Lighting
   float dot3Factor = max(dot(passPosition, dirToLight), 0.0f) * attenuation;
   vec3 dot3Color = lightColor * dot3Factor;

   // Specular Lighting
   float specFactor = max(dot(normal, halfVector), 0.0f);
   specFactor = pow(specFactor, specPower) * specIntensity * attenuation;
   vec3 specColor = lightColor * specFactor;

   // Return it
   SurfaceLightColors returnColors;
   returnColors.surfaceColor = dot3Color;
   returnColors.specularColor = specColor;
   return returnColors;
}

//------------------------------------------------------------------------
void DissolveOverTime(void)
{
  vec3 noise = texture(gNoiseTexture, passUV0).rgb;
  float noiseMagnitude = sqrt(dot(noise, noise));
  if(gDissolveAmount > noiseMagnitude)
  {
    discard;
  }
  if(gDissolveAmount + 0.05f > noiseMagnitude)
  {
    outColor = gDissolveColor;
  }
}

//------------------------------------------------------------------------
void main(void)
{
  vec3 surfaceTangent = normalize(passTangent);
  vec3 surfaceBitangent = normalize(passBitangent);
  vec3 surfaceNormal = cross(surfaceBitangent, surfaceTangent);
  surfaceBitangent = cross(surfaceTangent, surfaceNormal);

  mat3 tbn = mat3(surfaceTangent, surfaceBitangent, surfaceNormal);
  tbn = transpose(tbn);

  vec4 diffuse = texture(gDiffuseTexture, passUV0);
  vec3 normalMap = texture(gNormalTexture, passUV0).rgb;
  vec4 emissive = texture(gEmissiveTexture, passUV0);

  vec3 normal = normalize((normalMap * vec3(2.0f, 2.0f, 1.0f)) - vec3(1.0f, 1.0f, 0.0f));
  normal = normal * tbn;

  vec3 light_intensity = vec3(0.0f);
  vec3 specularFactor = vec3(0.0f);
  for (int i = 0; i < gLightCount; ++i)
  {
    SurfaceLightColors colors = CalculateSurfaceLight(
       normal,
       gLightPosition[i],
       gLightDirection[i],
       gLightColor[i].rgb,
       gLightDirectionFactor[i],
       gSpecularPower,
       gSpecularIntensity,
       gNearDistance[i], gFarDistance[i],
       gNearPower[i], gFarPower[i],
       gInnerAngle[i], gOuterAngle[i],
       gInnerPower[i], gOuterPower[i]);
    light_intensity += colors.surfaceColor;
    specularFactor += colors.specularColor;
  }

  outColor = passColor * gColor * diffuse * vec4(light_intensity, 1.0f) + vec4(specularFactor, 1.0f) + vec4(emissive.rgb * emissive.a, 1.0f);
  outColor = clamp(outColor, vec4(0.0f), vec4(1.0f));
  float distanceToPixel = distance(passPosition, gCameraPosition);
  if(distanceToPixel > gMaxFogDistance)
  {
    outColor = vec4(gFogColor.rgb, outColor.a);
  }
  else if(distanceToPixel > gMinFogDistance)
  {
    float fogRatio = (distanceToPixel - gMinFogDistance) / (gMaxFogDistance - gMinFogDistance);
    float inverseFogRatio = 1.0f - fogRatio;
    outColor = vec4((gFogColor.rgb * fogRatio) + (outColor.rgb * inverseFogRatio), outColor.a);
  }
  DissolveOverTime();
}
  //Debug line, output UV's as colors.
  //outColor = vec4(passUV0, 0.0f, 1.0f);


  //vec3 normal = vec3(0.0f, 0.0f, -1.0f); //Whatever is forward for your texture.
  //ranges of a color (0-1, 0-1, 0-1, DON'T CARE)
  //maps to a range (-1 - 1, -1 - 1, 0 - 1);

  //DEBUG: See if both textures are loaded. Load half as one texture and half as the other.

  /*
  if(passUV0.x > .5f)
  {
    outColor = diffuse;
  }
  else
  {
    outColor = vec4(normalMap, 1.0f);
  }
  */


  //DEBUG: See the actual color of the light on a plain black/white texture.
  //outColor = vec4(light_intensity, 1.0f);

  //DEBUG: map position to color
  //vec4 color = vec4((passPosition.xy + vec2(2.0f)) / 4.0f, 0.0f, 1.0f);
  //outColor = color;
