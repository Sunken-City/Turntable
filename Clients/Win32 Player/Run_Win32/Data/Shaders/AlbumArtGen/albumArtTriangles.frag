#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture;
uniform int gHashVal;

in vec2 passUV0;
in vec3 passPosition;
in vec4 passColor;

out vec4 outColor;


vec2 getTriangleIndex(vec2 uv)
{
	float skewFactor = 0.5773503; //sqrt(3)/3

	//Skew the UVs so that when we use mod to make a "checkerboard", we end up with diamonds, not squares
	vec2 skewedUVs = vec2( uv.x * 2.0 * skewFactor, uv.y + uv.x * skewFactor );

	//The Integer portion of the UVs are a set of vertical stripes
	vec2 uvFrac = fract(skewedUVs);

	//Generates a grid of triangles
	vec2 triangles = step(uvFrac.xy, uvFrac.yx);

	return triangles;
}

void main(void)
{
	float scale = 6.0f;// * ((float(gHashVal % 3) + 1.0f) / 2.0f);
  vec2 uv = passUV0 * scale;

  vec2 hex = getTriangleIndex(uv);
  int squareNumber = int(hex.x) + int(hex.y * scale);

	float red = (gHashVal & 0xFF) / 256.0f;
	float green = ((gHashVal & 0xFF00) >> 8) / 256.0f;
	float blue = ((gHashVal & 0xFF0000) >> 16) / 256.0f;

	float red2 = ((gHashVal & 0xFF000000) >> 24) / 256.0f;
	float green2 = ((gHashVal & 0xFF000) >> 12) / 256.0f;
	float blue2 = ((gHashVal & 0xFF0) >> 4) / 256.0f;
  outColor = mix(vec4(red, green, blue, 1.0), vec4(red2, green2, blue2, 1.0), squareNumber % 2);//(squareNumber % (gHashVal % 10)) / scale);
}
