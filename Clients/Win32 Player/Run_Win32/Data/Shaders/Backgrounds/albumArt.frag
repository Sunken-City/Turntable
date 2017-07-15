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
  float scalingFactor = 0.5f;
  vec2 aspectRatio = vec2(16, 9) * scalingFactor;
  float timeFactor = gTime / 20.0f;

  vec2 uv = (passUV0 + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square
  uv.y *= -1.0f; //STBI correction


  outColor = texture(gNormalTexture, uv);
}
