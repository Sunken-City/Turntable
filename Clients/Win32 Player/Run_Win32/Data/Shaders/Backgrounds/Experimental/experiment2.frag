#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture; //gDepthTexture
uniform float gTime;

in vec2 passUV0;
in vec3 passPosition;

out vec4 outColor;

void main(void)
{
  vec2 offset = passUV0 - vec2(.5f);
  float d = length(offset) * 200.0f; //how far am I from the center

  vec2 uv = passUV0 - vec2(0.5f);
  float xSquared = uv.x * uv.x;
  float ySquared = uv.y * uv.y;
  float r = sqrt(xSquared + ySquared);
  float theta = atan(ySquared, xSquared);

  float red   = cos(length(offset * (theta + r)) * -20.0f + gTime + 0) * 0.5f + 0.5f;
  float green = cos(length(offset * (theta + r)) * -20.0f + gTime + 2) * 0.5f + 0.5f;
  float blue  = cos(length(offset * (theta + r)) * -20.0f + gTime + 4) * 0.5f + 0.5f;

  float color = length(offset) * 1.5f + (sin(theta * 10.0f + r + gTime * theta) * 0.1f);
  vec4 fragColor = vec4(color, color, color, 1.0f);

  outColor = fragColor;
}
