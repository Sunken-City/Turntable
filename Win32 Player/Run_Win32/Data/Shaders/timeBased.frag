#version 410 core

uniform vec4 gColor;
uniform sampler2D gDiffuseTex;
uniform float gTime;
uniform int gEffectState;

in vec4 passColor;
in vec2 passUV0;

out vec4 outColor;

void main(void)
{
  vec4 diffuse = texture(gDiffuseTex, passUV0);
  //Debug line, output UV's as colors.
  //outColor = vec4(passUV0, 0.0f, 1.0f);
  if(gEffectState == 1)
  {
      outColor = passColor * gColor * diffuse * sin(gTime);
  }
  else
  {
      outColor = passColor * gColor * diffuse;
  }
}
