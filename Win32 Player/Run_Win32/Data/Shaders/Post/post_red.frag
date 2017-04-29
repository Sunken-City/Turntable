#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture; //gDepthTexture

in vec2 passUV0;

out vec4 outColor;

void main(void)
{
  float red = texture(gDiffuseTexture, passUV0).r;

  float depth = texture(gNormalTexture, passUV0).r;

  outColor = vec4(red, 0, 0, 1);
}
