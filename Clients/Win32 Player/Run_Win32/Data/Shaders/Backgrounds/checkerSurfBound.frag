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
  vec4 earthboundBlue = vec4(0.407, 0.659, 0.847, 1.0);
  vec4 earthboundGreen = vec4(0.500, 0.847, 0.565, 1.0);
  vec2 aspectRatio = vec2(16, 9) * 3.0f;
  float timeFactor = gTime / 20.0f;

  vec2 uv = ((passUV0 * passUV0.x * passUV0.y) + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  int xCoordinate = int(round(uv.x));
  int yCoordinate = int(round(uv.y));
  int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

  squareNumber += yCoordinate % 2; //Change the colors every other row

  outColor = mix(earthboundBlue, earthboundGreen, squareNumber % 2);


}
