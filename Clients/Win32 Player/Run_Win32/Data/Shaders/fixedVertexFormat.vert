#version 410 core

uniform mat4 gModel;
uniform mat4 gView;
uniform mat4 gProj;

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV0;

out vec3 passPosition;
out vec4 passColor;
out vec2 passUV0;

void main(void)
{
  passColor = inColor;
  passUV0 = inUV0;

  vec4 pos = vec4(inPosition, 1.0f);
  passPosition = (pos * gModel).xyz;
  pos = pos * gModel * gView * gProj; //column major
  // row major would be:
  // pos = gProj * gView * gModel * pos; //Row major

  gl_Position = pos; //gl_Position is a built in type
}
