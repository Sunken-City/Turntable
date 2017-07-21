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
  // I'm not using this line
  // outColor = vec4(63.0/255.0, 66.0/255.0, 255.0/255.0, 1.0);

  /*
  float red = passUV0.x;
  float green = passUV0.y;
  float blue = 0.0;
  float alpha = 1.0;
  outColor = vec4(red, green, blue, alpha);
  */
  //outColor = vec4(0.0f, passUV0.x, passUV0.y, 1.0f);

  /*


  float red   = sin(gTime + 0) * 0.5f + 0.5f;
  float green = sin(gTime + 2) * 0.5f + 0.5f;
  float blue  = sin(gTime + 4) * 0.5f + 0.5f;

  vec4 rainbowColor = vec4(red, green, blue, 1.0f);*/

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

  uv = (passUV0 + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  int xCoordinate = int(round(uv.x));
  int yCoordinate = int(round(uv.y));
  int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

  squareNumber += yCoordinate % 2; //Change the colors every other row
  outColor = mix(textureColorAtUVCoordinate, earthboundOrange, squareNumber % 2);


}
