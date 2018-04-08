
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec4 yellow = vec4(1.0f, 0.85f, 0.0f, 1.0f);
  vec4 orange = vec4(1.0f, 0.57f, 0.0f, 1.0f);
  vec4 darkBlue = vec4(0.15f, 0.0f, 0.35f, 1.0f);
  float timeFactor = -iTime / 20.0f;
  int degreesPerBand = 30;

  vec2 uv = fragCoord/iResolution.xy;//(passUV0 + vec2(timeFactor, timeFactor));
  float xSquared = uv.x * uv.x;
  float ySquared = uv.y * uv.y;
  float r = sqrt(xSquared + ySquared);
  float theta = atan(ySquared, xSquared) - timeFactor;
  int thetaDegrees = int(theta * (180.0f / 3.14159f)) % 360;

  yellow = mix(darkBlue, yellow, 0.8f - uv.y);
  orange = mix(darkBlue, orange, 0.8f - uv.y);
  fragColor = mix(yellow, orange, thetaDegrees % 2);

  //if(uv.y > 0.75f && fract(uv.y * uv.x + uv.x / uv.y * 100000.0f + iTime) - 0.99f >= 0.001f)
  //{
  //  fragColor = vec4(1,1,1,1);
  //}
}
