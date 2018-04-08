
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 aspectRatio = vec2(16, 9) * 0.5f;
  float timeFactor = iTime / 20.0f;

  vec2 uv = (fragCoord/iResolution.xy + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square

  int xCoordinate = int(round(uv.x));
  int yCoordinate = int(round(uv.y));
  int squareNumber = xCoordinate + (int(aspectRatio.x) * yCoordinate);

  fragColor = mix(vec4(1.0f, 0.8f, 0.0f, 1.0f), vec4(vec3(0.0f), 1.0f), squareNumber % 2);
}
