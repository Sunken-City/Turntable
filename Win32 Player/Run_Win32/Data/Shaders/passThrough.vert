#version 410 core

in vec3 inPosition;

void main(void)
{
  gl_Position = vec4( inPosition, 1.0f); //gl_position is a built in type
}
