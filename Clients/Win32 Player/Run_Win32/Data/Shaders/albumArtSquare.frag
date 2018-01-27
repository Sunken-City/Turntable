#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture;
uniform int gHashVal;

in vec2 passUV0;
in vec3 passPosition;
in vec4 passColor;

out vec4 outColor;

void main(void)
{
	int scale = 8;
  vec2 uv = passUV0 * scale;

	int xCoordinate = int(round(uv.x));
  int yCoordinate = int(round(uv.y));
  int squareNumber = xCoordinate + (scale * yCoordinate);

  squareNumber += yCoordinate % 2; //Change the colors every other row

	float red = (gHashVal & 0xFF) / 256.0f;
	float green = ((gHashVal & 0xFF00) >> 8) / 256.0f;
	float blue = ((gHashVal & 0xFF0000) >> 16) / 256.0f;

	float red2 = ((gHashVal & 0xFF000000) >> 24) / 256.0f;
	float green2 = ((gHashVal & 0xFF000) >> 12) / 256.0f;
	float blue2 = ((gHashVal & 0xFF0) >> 4) / 256.0f;
  outColor = mix(vec4(red, green, blue, 1.0), vec4(red2, green2, blue2, 1.0), (squareNumber % (gHashVal % 10)) / 8.0f);
}
