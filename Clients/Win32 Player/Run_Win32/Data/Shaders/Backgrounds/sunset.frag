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
  vec4 yellow = vec4(1.0f, 0.85f, 0.0f, 1.0f);
  vec4 orange = vec4(1.0f, 0.57f, 0.0f, 1.0f);
  vec4 darkBlue = vec4(0.15f, 0.0f, 0.35f, 1.0f);
  float timeFactor = -gTime / 20.0f;
  int degreesPerBand = 30;

  vec2 uv = passUV0;//(passUV0 + vec2(timeFactor, timeFactor));
  float xSquared = uv.x * uv.x;
  float ySquared = uv.y * uv.y;
  float r = sqrt(xSquared + ySquared);
  float theta = atan(ySquared, xSquared) - timeFactor;
  int thetaDegrees = int(theta * (180.0f / 3.14159f)) % 360;

  yellow = mix(darkBlue, yellow, 0.8f - passUV0.y);
  orange = mix(darkBlue, orange, 0.8f - passUV0.y);
  outColor = mix(yellow, orange, thetaDegrees % 2);

  //if(uv.y > 0.75f && fract(uv.y * uv.x + uv.x / uv.y * 100000.0f + gTime) - 0.99f >= 0.001f)
  //{
  //  outColor = vec4(1,1,1,1);
  //}
}
