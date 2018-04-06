#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture;

uniform vec4 gColor;
uniform vec4 gAmbientLight;
uniform vec4 gLightColor;
uniform vec4 gFogColor;

uniform vec3 gLightPosition;
uniform vec3 gCameraPosition;

uniform float gLightIntensity;
uniform float gSpecularPower;
uniform float gMinFogDistance;
uniform float gMaxFogDistance;
uniform float gTime;

in vec4 passColor;
in vec2 passUV0;
in vec3 passPosition;
in vec3 passNormal;

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
  float specularIntensity = texture(gNormalTexture, passUV0).r;

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

void main(void)
{
  vec3 normal = passNormal;

  vec4 diffuse = texture(gDiffuseTexture, passUV0);

  vec3 light_intensity = CalculateLightFactor(normal);
  vec3 specularFactor = CalculateSpecularFactor(normal);

  outColor = passColor * diffuse * vec4(light_intensity, 1.0f) + vec4(specularFactor, 1.0f);
  //outColor = clamp(outColor, vec4(0.0f), vec4(1.0f));
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

  //outColor += vec4(light_intensity, 1.0f);
}

  //DEBUG: See the actual color of the light on a plain black/white texture.
  //outColor = vec4(light_intensity, 1.0f);

  //DEBUG: map position to color
  //vec4 color = vec4((passPosition.xy + vec2(2.0f)) / 4.0f, 0.0f, 1.0f);
  //outColor = color;
