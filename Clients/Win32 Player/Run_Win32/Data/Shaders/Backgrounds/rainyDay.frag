
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 passUV = fragCoord/iResolution.xy;
  vec4 lightBlue = vec4(0.49, 0.796, 0.745, 1.0);
  vec4 darkBlue = vec4(0.0, 0.224, 0.431, 1.0);
  float timeFactor = iTime / 20.0f;
  vec2 uv = (passUV + vec2(timeFactor, timeFactor)) * vec2(16, 9);
  if(abs(fract(uv.y) - fract(uv.x)) < 0.1f)
  {
    fragColor = lightBlue;
  }
  else
  {
    fragColor = darkBlue;
  }
}
