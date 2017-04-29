#version 410 core

uniform vec4 gColor;

in vec4 passColor;

out vec4 outColor;

void main(void)
{
  outColor = gColor;
}
