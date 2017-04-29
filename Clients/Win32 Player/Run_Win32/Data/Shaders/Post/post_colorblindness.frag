#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture; //gDepthTexture

in vec2 passUV0;

out vec4 outColor;

//Derived from https://gist.github.com/jcdickinson/580b7fb5cc145cee8740
void main(void)
{
  vec4 diffuse = texture(gDiffuseTexture, passUV0);
  float L = (17.8824f * diffuse.r) + (43.5161f * diffuse.g) + (4.11935f * diffuse.b);
	float M = (3.45565f * diffuse.r) + (27.1554f * diffuse.g) + (3.86714f * diffuse.b);
	float S = (0.0299566f * diffuse.r) + (0.184309f * diffuse.g) + (1.46709f * diffuse.b);

  float l, m, s;

  if(passUV0.x < 0.25f)
  {
    outColor = diffuse;
  }
  else if(passUV0.x < 0.50f)
  {
    //Protanopia
    l = 0.0f * L + 2.02344f * M + -2.52581f * S;
    m = 0.0f * L + 1.0f * M + 0.0f * S;
    s = 0.0f * L + 0.0f * M + 1.0f * S;
    outColor.r = (0.0809444479f * l) + (-0.130504409f * m) + (0.116721066f * s);
    outColor.g = (-0.0102485335f * l) + (0.0540193266f * m) + (-0.113614708f * s);
    outColor.b = (-0.000365296938f * l) + (-0.00412161469f * m) + (0.693511405f * s);
    outColor.a = 1;
  }
  else if(passUV0.x < 0.75f)
  {
    //Deuteranopia
    l = 1.0f * L + 0.0f * M + 0.0f * S;
    m = 0.494207f * L + 0.0f * M + 1.24827f * S;
    s = 0.0f * L + 0.0f * M + 1.0f * S;
    outColor.r = (0.0809444479f * l) + (-0.130504409f * m) + (0.116721066f * s);
    outColor.g = (-0.0102485335f * l) + (0.0540193266f * m) + (-0.113614708f * s);
    outColor.b = (-0.000365296938f * l) + (-0.00412161469f * m) + (0.693511405f * s);
    outColor.a = 1;
  }
  else if(passUV0.x <= 1.0f)
  {
    //Tritanopia
    l = 1.0f * L + 0.0f * M + 0.0f * S;
    m = 0.0f * L + 1.0f * M + 0.0f * S;
    s = -0.395913f * L + 0.801109f * M + 0.0f * S;
    outColor.r = (0.0809444479f * l) + (-0.130504409f * m) + (0.116721066f * s);
    outColor.g = (-0.0102485335f * l) + (0.0540193266f * m) + (-0.113614708f * s);
    outColor.b = (-0.000365296938f * l) + (-0.00412161469f * m) + (0.693511405f * s);
    outColor.a = 1;
  }
}
