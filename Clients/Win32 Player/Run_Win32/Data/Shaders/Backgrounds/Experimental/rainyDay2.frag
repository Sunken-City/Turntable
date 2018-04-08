
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 passUV = fragCoord/iResolution.xy;
  vec4 lightBlue = vec4(0.69, 0.696, 0.745, 1.0);
  vec4 darkBlue = vec4(0.69, 0.424, 0.56, 1.0);
  float timeFactor = iTime / 20.0f;
  vec2 uv = (((passUV + vec2(timeFactor, timeFactor) * 100) * vec2(10, 10)) * (passUV.y / 5));
  if(abs(fract(uv.y) - fract(uv.x)) < 0.2f)
  {
    fragColor = lightBlue;
  }
  else
  {
    fragColor = darkBlue;
  }
}
