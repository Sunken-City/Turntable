#version 410 core

uniform mat4 gModel;
uniform mat4 gView;
uniform mat4 gProj;

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV0;

out vec4 passColor;
out vec2 passUV0;

void main(void)
{
  passColor = inColor;
  passUV0 = inUV0;

  vec4 pos = vec4(inPosition, 1.0f);
  pos = pos * gModel; //* gView * gProj;

  vec4 fixedPosition = vec4(0.0f, 0.0f, 0.5f, 1.0f);
  float distanceToPosition = length(pos - fixedPosition);
  vec4 finalPosition = mix(fixedPosition, pos, 1.0f - distanceToPosition);
  finalPosition = finalPosition * gView * gProj;

  gl_Position = pos; //gl_position is a built in type
}
