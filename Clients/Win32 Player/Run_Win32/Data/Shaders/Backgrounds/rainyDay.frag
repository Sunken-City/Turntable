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
  vec4 lightBlue = vec4(0.49, 0.796, 0.745, 1.0);
  vec4 darkBlue = vec4(0.0, 0.224, 0.431, 1.0);
  float timeFactor = gTime / 20.0f;
  vec2 uv = (passUV0 + vec2(timeFactor, timeFactor)) * vec2(16, 9);
  if(abs(fract(uv.y) - fract(uv.x)) < 0.1f)
  {
    outColor = lightBlue;
  }
  else
  {
    outColor = darkBlue;
  }
}
