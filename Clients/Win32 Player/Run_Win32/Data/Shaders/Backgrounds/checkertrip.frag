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
  float slowDownFactor = 20.0f;
  float timeFactor = gTime / slowDownFactor;

  vec2 uv = (passUV0 + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  aspectRatio = vec2(16, 9) * 3.0f;
  timeFactor = gTime / 20.0f;

  float first = mix(passUV0.x, passUV0.y, sin(gTime * 0.14159f) * 0.5f + 0.5f);
  float second = mix(1.0f - passUV0.x, 1.0f - passUV0.y, sin(gTime * 0.59141f) * 0.5f + 0.5f);
  uv = ((passUV0 * mix(first, second, sin(gTime) * 0.5 + 0.5)) + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  int xCoordinate = int(round(uv.x));
  int yCoordinate = int(round(uv.y));
  int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

  squareNumber += yCoordinate % 2; //Change the colors every other row
  outColor = mix(vec4(1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), squareNumber % 2);


}
