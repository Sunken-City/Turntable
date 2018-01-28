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
  vec2 aspectRatio = vec2(16, 9) * 3.0f;
  float timeFactor = gTime * 10.0f;
  int degreesPerBand = 30;

  vec2 uv = passUV0 - vec2(0.5f);
  uv = uv * aspectRatio; //Multiply by the aspect ratio to make our shader square

  float xSquared = uv.x * uv.x;
  float ySquared = uv.y * uv.y;
  float theta = atan(ySquared, xSquared);
  int thetaDegrees = int(theta * (180.0f / 3.14159f));
  if(passUV0.x < 0.5f)
  {
    thetaDegrees = 180 - thetaDegrees;
  }
  if(passUV0.y < 0.5f)
  {
    thetaDegrees = 360 - thetaDegrees;
  }

  thetaDegrees = int(float(thetaDegrees) + timeFactor) % 360;

  //outColor = mix(earthboundGreen, earthboundBlue, (thetaDegrees / degreesPerBand) % 2);
  outColor = vec4(float(thetaDegrees) / 360.0f, float(thetaDegrees) / 360.0f, float(thetaDegrees) / 360.0f, 1.0f);
}
