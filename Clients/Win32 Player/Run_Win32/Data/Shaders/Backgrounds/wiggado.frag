
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 passUV = fragCoord/iResolution.xy;
  vec2 aspectRatio = vec2(16, 9) * 3.0f;
  float timeFactor = iTime / 20.0f;

  vec2 uv = ((passUV * passUV.x * passUV.y * (1.0f-passUV.x) * (1.0f-passUV.y)) + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  int xCoordinate = int(round(uv.x));
  int yCoordinate = int(round(uv.y));
  int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

  squareNumber += yCoordinate % 2; //Change the colors every other row

  float red   = sin(iTime + passUV.x + passUV.y + 0) * 0.5f + 0.5f;
  float green = sin(iTime + passUV.x + passUV.y + 2) * 0.5f + 0.5f;
  float blue  = sin(iTime + passUV.x + passUV.y + 4) * 0.5f + 0.5f;

  vec4 rainbowColor = vec4(red, green, blue, 1.0f);
  vec4 invertedRainbowColor = vec4(1.0f - red, 1.0f - green, 1.0f - blue, 1.0f);

  fragColor = mix(rainbowColor, invertedRainbowColor, squareNumber % 2);


}
