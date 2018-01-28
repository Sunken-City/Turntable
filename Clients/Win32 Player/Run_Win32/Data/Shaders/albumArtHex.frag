#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture;
uniform int gHashVal;

in vec2 passUV0;
in vec3 passPosition;
in vec4 passColor;

out vec4 outColor;


vec2 getHexagonIndex(vec2 uv)
{
	float skewFactor = 0.5773503; //sqrt(3)/3

	//Skew the UVs so that when we use mod to make a "checkerboard", we end up with diamonds, not squares
	vec2 skewedUVs = vec2( uv.x * 2.0 * skewFactor, uv.y + uv.x * skewFactor );

	//The Integer portion of the UVs are a set of vertical stripes
	vec2 uvInt = floor(skewedUVs);
	vec2 uvFrac = fract(skewedUVs);

	//Create a chain of diamonds, each filling up 1/3 of a hexagon
	float diamonds1 = mod(uvInt.x + uvInt.y, 3.0);

	//Create a chain of diamonds, each filling up 1/3 of a hexagon, adjacent to the first and slightly offset
	float diamonds2 = step(2.0, diamonds1);

	//Create an inverted chain of diamonds, conveniently equivalent to a combination of both above chains
	float bothDiamonds = step(1.0, diamonds1);

	//Generates a grid of triangles
	vec2 triangles = step(uvFrac.xy, uvFrac.yx);

	//Isolates out 1/3 of the triangles
	vec2 isolatedTriangles = diamonds2 * triangles;

	//Combining the vertical stripes and the diamonds creates a zig-zag pattern
	vec2 zigZag = uvInt + bothDiamonds;

	//This zig-zag pattern is close to our hexagons, but there's 1/6th of the hexagon that's translated.
	//Luckily, this matches up with our isolated triangle pattern from before, and by subtracting them,
	//we manage to invert the colors of that last 1/6th, completing both the hexagonal regions.
	//We now have the coordinates of each hexagon in the image.
	return vec2(zigZag - isolatedTriangles);
}

void main(void)
{
	float scale = 8.0f;
  vec2 uv = passUV0 * 8.0f;

  vec2 hex = getHexagonIndex(uv);
  int squareNumber = int(hex.x) + int(hex.y * scale);

	float red = (gHashVal & 0xFF) / 256.0f;
	float green = ((gHashVal & 0xFF00) >> 8) / 256.0f;
	float blue = ((gHashVal & 0xFF0000) >> 16) / 256.0f;

	float red2 = ((gHashVal & 0xFF000000) >> 24) / 256.0f;
	float green2 = ((gHashVal & 0xFF000) >> 12) / 256.0f;
	float blue2 = ((gHashVal & 0xFF0) >> 4) / 256.0f;
  outColor = mix(vec4(red, green, blue, 1.0), vec4(red2, green2, blue2, 1.0), (squareNumber % (gHashVal % 10)) / 8.0f);
}
