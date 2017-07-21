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
  uv.y *= -1.0f; //STBI correction
  vec4 textureColorAtUVCoordinate = texture(gNormalTexture, uv);

  vec4 earthboundBlue = vec4(0.407, 0.7, 0.847, 1.0);
  vec4 earthboundOrange = vec4(255.0/255.0, 131.0/255.0, 107.0/255.0, 1.0);
  aspectRatio = vec2(16, 9) * 3.0f;
  timeFactor = gTime / 20.0f;

  uv = ((passUV0 * passUV0.x * passUV0.y * (1.0f-passUV0.x) * (1.0f-passUV0.y)) + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  int xCoordinate = int(round(uv.x));
  int yCoordinate = int(round(uv.y));
  int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

  squareNumber += yCoordinate % 2; //Change the colors every other row
  outColor = mix(textureColorAtUVCoordinate, vec4(0.0f, 0.0f, 0.0f, 1.0f), squareNumber % 2);


}
