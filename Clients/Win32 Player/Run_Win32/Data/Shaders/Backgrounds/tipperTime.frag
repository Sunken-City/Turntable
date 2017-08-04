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
  int degreesPerBand = 30;

  vec2 uv = passUV0 - vec2(0.5f);
  float xSquared = uv.x * uv.x;
  float ySquared = uv.y * uv.y;
  float r = sqrt(xSquared + ySquared);
  float theta = atan(ySquared, xSquared) + timeFactor;
  int thetaDegrees = int(theta * (180.0f / 3.14159f));

  outColor = mix(earthboundGreen, earthboundBlue, thetaDegrees % degreesPerBand);
}
