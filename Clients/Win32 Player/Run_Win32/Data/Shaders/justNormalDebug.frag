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
in vec3 passNormal;

out vec4 outColor;

vec4 VectorAsColor(vec3 vec)
{
    return vec4((vec + vec3(1.0f)) * vec3(.5f), 1.0f);
}

void main(void)
{
  vec3 normal = passNormal;

  outColor = VectorAsColor(normal);
}
