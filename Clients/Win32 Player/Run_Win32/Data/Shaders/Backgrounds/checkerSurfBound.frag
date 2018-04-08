
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec4 earthboundBlue = vec4(0.407, 0.659, 0.847, 1.0);
  vec4 earthboundGreen = vec4(0.500, 0.847, 0.565, 1.0);
  vec2 aspectRatio = vec2(16, 9) * 3.0f;
  float timeFactor = iTime / 20.0f;

  vec2 passUV = fragCoord/iResolution.xy;
  
  vec2 uv = ((passUV * passUV.x * passUV.y) + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  int xCoordinate = int(round(uv.x));
  int yCoordinate = int(round(uv.y));
  int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

  squareNumber += yCoordinate % 2; //Change the colors every other row

  fragColor = mix(earthboundBlue, earthboundGreen, squareNumber % 2);


}
