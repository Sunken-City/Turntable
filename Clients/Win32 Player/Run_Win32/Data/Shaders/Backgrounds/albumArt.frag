void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  float scalingFactor = 0.5f;
  vec2 aspectRatio = vec2(16, 9) * scalingFactor;
  float slowDownFactor = 20.0f;
  float timeFactor = iTime / slowDownFactor;

  vec2 uv = (fragCoord/iResolution.xy + vec2(timeFactor, timeFactor));
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our checkerboard squares square
  uv.y *= -1.0f; //STBI correction

  fragColor = texture(iChannel0, uv);
}
