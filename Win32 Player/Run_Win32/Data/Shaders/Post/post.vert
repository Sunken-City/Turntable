#version 410 core

uniform mat4 gModel;
uniform mat4 gView;
uniform mat4 gProj;

in vec3 inPosition;
in vec2 inUV0;

out vec2 passUV0;
out vec3 passPosition;

void main(void)
{
  //Recorrect for inverted texture coords
  passUV0 = vec2(inUV0.x, 1.0f - inUV0.y);
  passPosition = inPosition;

  vec4 pos = vec4(inPosition, 1.0f);
  pos = pos * gModel * gView * gProj;

  gl_Position = pos; //gl_position is a built in type
}
