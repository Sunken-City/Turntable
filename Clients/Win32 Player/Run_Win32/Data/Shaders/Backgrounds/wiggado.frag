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
  vec2 aspectRatio = vec2(16, 9) * 3.0f;
  float timeFactor = gTime / 20.0f;

  vec2 uv = ((passUV0 * passUV0.x * passUV0.y * (1.0f-passUV0.x) * (1.0f-passUV0.y)) + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  int xCoordinate = int(round(uv.x));
  int yCoordinate = int(round(uv.y));
  int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

  squareNumber += yCoordinate % 2; //Change the colors every other row

  float red   = sin(gTime + passPosition.x + passPosition.y + 0) * 0.5f + 0.5f;
  float green = sin(gTime + passPosition.x + passPosition.y + 2) * 0.5f + 0.5f;
  float blue  = sin(gTime + passPosition.x + passPosition.y + 4) * 0.5f + 0.5f;

  vec4 rainbowColor = vec4(red, green, blue, 1.0f);
  vec4 invertedRainbowColor = vec4(1.0f - red, 1.0f - green, 1.0f - blue, 1.0f);

  outColor = mix(rainbowColor, invertedRainbowColor, squareNumber % 2);


}
