
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 uv = fragCoord/iResolution.xy;
  float remainder = fract(uv.y * uv.x + uv.x / uv.y * 0.01f + iTime) - 0.9f;
  if(remainder >= 0.01f)
  {
    fragColor = vec4(0.84, 0.98, 0.99, 1.0);//mix(vec4(1,1,1,1), vec4(1,0,1,1), abs((uv.x - 0.8f) * 8.0f) * abs((uv.y - 0.5f) * 8.0f));
  }
  else
  {
    fragColor = vec4(0.0, 0.44, 0.73, 1.0);
  }
}
