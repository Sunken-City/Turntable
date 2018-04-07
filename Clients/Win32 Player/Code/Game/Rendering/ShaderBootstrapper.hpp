#pragma once

class ShaderProgram;

//Toolkit used to bootstrap and load shaders following the ShaderToy format
class ShaderBootstrapper
{
public:
	static ShaderProgram* compileShader(const char* vertexShader, const char* fragmentShader);

private:
	static const char* mainFunction;
};