
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 passUV = fragCoord/iResolution.xy;
  vec2 offset = passUV - vec2(.5f);
  float d = length(offset) * 200.0f; //how far am I from the center
  vec2 u = offset / d;

  float offsetAmount = .05f * -cos((iTime * 4.0f) + (d * 100.0f));
  vec2 uv = passUV + u * offsetAmount;

  float red   = sin(iTime + passUV.x + passUV.y + 0) * 0.5f + 0.5f;
  float green = sin(iTime + passUV.x + passUV.y + 2) * 0.5f + 0.5f;
  float blue  = sin(iTime + passUV.x + passUV.y + 4) * 0.5f + 0.5f;

  vec4 rainbowColor = vec4(red, green, blue, 1.0f);

  fragColor = rainbowColor;
}
