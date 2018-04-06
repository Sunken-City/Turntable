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
  vec4 earthboundBlue = vec4(0.407, 0.659, 0.847, 1.0);
  vec4 earthboundGreen = vec4(0.500, 0.847, 0.565, 1.0);
  float timeFactor = gTime / 20.0f;
  vec2 uv = (passUV0 + vec2(timeFactor, timeFactor)) * vec2(16, 9);
  if(abs(fract(uv.y) - fract(uv.x)) < 0.1f)
  {
    outColor = earthboundBlue;
  }
  else
  {
    outColor = earthboundGreen;
  }
}
