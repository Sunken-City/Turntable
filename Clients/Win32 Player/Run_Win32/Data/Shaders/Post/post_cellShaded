#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture; //gDepthTexture

in vec2 passUV0;

out vec4 outColor;

void main(void)
{
  vec4 diffuse = texture(gDiffuseTexture, passUV0);
  diffuse = diffuse * 3;
  diffuse = floor(diffuse);
  diffuse = diffuse / 3;

  float depthHere = texture(gNormalTexture, passUV0).r;
  float scale = 1.0f - (depthHere * depthHere);
  float depthRight = texture(gNormalTexture, passUV0 + scale * vec2(.05f, 0.0f)).r;
  float depthLeft = texture(gNormalTexture, passUV0 - scale * vec2(.05f, 0.0f)).r;

  depthLeft = depthLeft * depthLeft;
  depthRight = depthRight * depthRight;
  float diff = abs(depthRight - depthLeft);

  if(diff > 0.01f)
  {
    outColor = vec4(0, 0, 0, 1);
  }
  else
  {
    outColor = vec4(1);
  }

  outColor = outColor * diffuse;
}
