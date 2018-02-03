#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture;
uniform float gTime;
uniform float gPixelationFactor;

in vec2 passUV0;
in vec3 passPosition;

out vec4 outColor;

void main(void)
{
  vec2 uv = passUV0;
  float remainder = fract(uv.y * uv.x + uv.x / uv.y * 10.0f + gTime) - 0.9f;
  if(remainder >= 0.01f)
  {
    outColor = mix(vec4(1,1,1,1), vec4(0.3,0,0.5,1), abs((uv.x - 0.8f) * 8.0f) * abs((uv.y - 0.5f) * 8.0f));
  }
  else
  {
    outColor = vec4(0,0,0,1);
  }
}
