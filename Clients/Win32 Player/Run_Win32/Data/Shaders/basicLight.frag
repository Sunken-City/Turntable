#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture;
uniform sampler2D gEmissiveTexture;
uniform sampler2D gNoiseTexture;
uniform sampler2D gTexSpecular;

uniform vec4 gColor;
uniform vec4 gAmbientLight;
uniform vec4 gLightColor;
uniform vec4 gFogColor;
uniform vec4 gDissolveColor;

uniform vec3 gLightPosition;
uniform vec3 gCameraPosition;

uniform float gLightIntensity;
uniform float gSpecularPower;
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

vec4 VectorAsColor(vec3 vec)
{
    return vec4((vec + vec3(1.0f)) * vec3(.5f), 1.0f);
}

vec3 CalculateLightFactor(vec3 normal)
{
    vec3 light_intensity = gAmbientLight.rgb * gAmbientLight.a;
    //different basis? move them to what you want
    //x -> your right
    //y -> your up
    //z -> your -forward
    vec3 vecToLight = gLightPosition - passPosition;
    float invLen = dot(vecToLight, vecToLight);
    invLen = inversesqrt(invLen);

    vecToLight = normalize(vecToLight);
    float power = gLightIntensity * (invLen * invLen);
    float nDotL = clamp(dot(normal, vecToLight), 0.0f, 1.0f);
    light_intensity += vec3(nDotL) * gLightColor.rgb * power;

    light_intensity = clamp(light_intensity, vec3(0.0f), vec3(1.0f)); //Saturate in DirectX
    return light_intensity;
}

vec3 CalculateSpecularFactor(vec3 normal)
{
  float specularIntensity = texture(gTexSpecular, passUV0).r;

  vec3 vecToLight = normalize(gLightPosition - passPosition);
  vec3 vecToEye = normalize(gCameraPosition - passPosition);
  vec3 halfVector = vecToLight + vecToEye; //Get the vector halfway between the two vectors
  halfVector = normalize(halfVector);

  float halfDotNormal = clamp(dot(halfVector, normal), 0.0f, 1.0f);
  float intensity = pow(halfDotNormal, gSpecularPower) * specularIntensity * 0.5f;

  float power = dot(vecToLight, vecToLight);
  power = clamp(gLightIntensity / power, 0.0f, 1.0f);

  vec4 color = intensity * gLightColor * power;
  return color.rgb;
}

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

  vec3 light_intensity = CalculateLightFactor(normal);
  vec3 specularFactor = CalculateSpecularFactor(normal);

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