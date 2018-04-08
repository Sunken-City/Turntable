
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 passUV = fragCoord/iResolution.xy;
  vec2 offset = passUV - vec2(.5f);
  float d = length(offset) * 200.0f; //how far am I from the center

  vec2 uv = passUV - vec2(0.5f);
  float xSquared = uv.x * uv.x;
  float ySquared = uv.y * uv.y;
  float r = sqrt(xSquared + ySquared);
  float theta = atan(ySquared, xSquared);

  float red   = cos(length(offset) * theta * -20.0f + iTime + 0) * 0.5f + 0.5f;
  float green = cos(length(offset) * theta * -20.0f + iTime + 2) * 0.5f + 0.5f;
  float blue  = cos(length(offset) * theta * -20.0f + iTime + 4) * 0.5f + 0.5f;

  fragColor = vec4(red, green, blue, 1.0f);
}
