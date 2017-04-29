#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture; //gDepthTexture
uniform float gTime;

in vec2 passUV0;
in vec4 passColor;

out vec4 outColor;

void main(void)
{
  vec2 offset = passUV0 - vec2(.5f);
  float d = length(offset); //how far am I from the center
  vec2 u = offset / d;

  float offsetAmount = .05f * -cos((gTime * 4.0f) + (d * 100.0f));
  vec2 uv = passUV0 + u * offsetAmount;

  outColor = texture(gDiffuseTexture, uv);
}
