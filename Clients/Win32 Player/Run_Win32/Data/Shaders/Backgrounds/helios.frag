
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec4 yellow = vec4(1.0f, 0.85f, 0.0f, 1.0f);
  vec4 orange = vec4(1.0f, 0.57f, 0.0f, 1.0f);
  float timeFactor = iTime / 20.0f;
  int degreesPerBand = 30;

  vec2 uv = fragCoord/iResolution.xy - vec2(0.5f);
  float xSquared = uv.x * uv.x;
  float ySquared = uv.y * uv.y;
  float r = sqrt(xSquared + ySquared);
  float theta = atan(ySquared, xSquared) + timeFactor;
  int thetaDegrees = int(theta * (180.0f / 3.14159f));

  fragColor = mix(yellow, orange, thetaDegrees % 2);
}
