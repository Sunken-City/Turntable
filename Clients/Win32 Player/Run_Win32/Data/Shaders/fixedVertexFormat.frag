#version 410 core

uniform vec4 gColor;
uniform sampler2D gDiffuseTexture;

in vec4 passColor;
in vec2 passUV0;

out vec4 outColor;

void main(void)
{
  vec4 diffuse = texture(gDiffuseTexture, passUV0);
  //Debug line, output UV's as colors.
  //outColor = vec4(passUV0, 0.0f, 1.0f);
  outColor = passColor * diffuse;
}
