#version 410 core

uniform mat4 gModel;
uniform mat4 gView;
uniform mat4 gProj;

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV0;
in vec3 inNormal;

out vec4 passColor;
out vec2 passUV0;
out vec3 passPosition;
out vec3 passNormal;

void main(void)
{
  vec4 pos = vec4(inPosition, 1.0f);
  passPosition = (pos * gModel).xyz;

  //Pass Normal to the FragmentShader
  passNormal = (vec4(inNormal, 0.0f) * gModel).xyz;

  pos = pos * gModel * gView * gProj; //column major
  // row major would be:
  // pos = gProj * gView * gModel * pos; //Row major

  passColor = inColor;
  passUV0 = inUV0;
  gl_Position = pos; //gl_position is a built in type
}
