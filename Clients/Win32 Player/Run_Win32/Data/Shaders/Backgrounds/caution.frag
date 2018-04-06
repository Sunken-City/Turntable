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
  vec2 aspectRatio = vec2(16, 9) * 0.5f;
  float timeFactor = gTime / 20.0f;

  vec2 uv = (passUV0 + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  int xCoordinate = int(round(uv.x + (uv.y / 2.0f)));
  int yCoordinate = int(round(uv.y + (uv.x / 2.0f)));
  int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

  outColor = mix(vec4(1.0f, 0.95f, 0.0f, 1.0f), vec4(vec3(0.0f), 1.0f), squareNumber % 2);
}
