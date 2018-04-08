
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  float red   = sin(iTime + 0) * 0.5f + 0.5f;
  float green = sin(iTime + 2) * 0.5f + 0.5f;
  float blue  = sin(iTime + 4) * 0.5f + 0.5f;

  vec4 rainbowColor = vec4(red, green, blue, 1.0f);

  fragColor = rainbowColor;
}
