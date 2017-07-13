#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture;
uniform float gTime;
uniform float gPixelationFactor;

in vec2 passUV0;
in vec3 passPosition;

out vec4 outColor;

void main(void)
{
  float red   = sin(gTime + 0) * 0.5f + 0.5f;
  float green = sin(gTime + 2) * 0.5f + 0.5f;
  float blue  = sin(gTime + 4) * 0.5f + 0.5f;

  vec4 rainbowColor = vec4(red, green, blue, 1.0f);

  outColor = rainbowColor;
}
