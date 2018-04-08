
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 passUV = fragCoord/iResolution.xy;
  vec4 yellow = vec4(1.0f, 0.85f, 0.0f, 1.0f);
  vec4 orange = vec4(1.0f, 0.57f, 0.0f, 1.0f);
  vec4 darkRed = vec4(0.5f, 0.0f, 0.0f, 1.0f);
  float timeFactor = iTime / 20.0f;
  int degreesPerBand = 30;

  float xSquared = passUV.x * passUV.x;
  float ySquared = passUV.y * passUV.y;
  float r = sqrt(xSquared + ySquared);
  float theta = atan(ySquared, xSquared) - timeFactor;
  int thetaDegrees = int(theta * (180.0f / 3.14159f)) % 360;

  yellow = mix(darkRed, yellow, passUV.y);
  orange = mix(darkRed, orange, passUV.y);
  fragColor = mix(yellow, orange, thetaDegrees % 2);
}
