
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  float scalingFactor = 0.5f;
  vec2 aspectRatio = vec2(16, 9) * scalingFactor;
  float slowDownFactor = 20.0f;
  float timeFactor = iTime / slowDownFactor;

  vec2 passUV = fragCoord/iResolution.xy;
  
  vec2 uv = (passUV + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  aspectRatio = vec2(16, 9) * 3.0f;
  timeFactor = iTime / 20.0f;

  float first = mix(passUV.x, passUV.y, sin(iTime * 0.14159f) * 0.5f + 0.5f);
  float second = mix(1.0f - passUV.x, 1.0f - passUV.y, sin(iTime * 0.59141f) * 0.5f + 0.5f);
  uv = ((passUV * mix(first, second, sin(iTime) * 0.5 + 0.5)) + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  int xCoordinate = int(round(uv.x));
  int yCoordinate = int(round(uv.y));
  int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

  squareNumber += yCoordinate % 2; //Change the colors every other row
  fragColor = mix(vec4(1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), squareNumber % 2);


}
