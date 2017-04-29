#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture; //gDepthTexture

in vec2 passUV0;

out vec4 outColor;

void main(void)
{
  vec4 diffuse = texture(gDiffuseTexture, passUV0);

  vec4 toGray = vec4(.2126, .7152, .0722, 0.0);
  float gray = dot(diffuse, toGray);

  vec3 r = vec3(.90, .29, .19);
  vec3 p = vec3(0.0, .20, .33);
  vec3 l = vec3(.95, .8, .7);
  vec3 b = vec3(.4, .7, .8);

  vec3 color;
  if(gray < .1)
  {
    color = p;
  }
  else if(gray < .4)
  {
    color = r;
  }
  else if(gray < .7)
  {
    color = b;
  }
  else
  {
    color = l;
  }

  outColor = vec4(color, 1.0);
}
