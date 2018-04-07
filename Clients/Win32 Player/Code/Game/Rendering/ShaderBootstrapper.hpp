#pragma once

class ShaderProgram;

//Toolkit used to bootstrap and load shaders following the ShaderToy format
class ShaderBootstrapper
{
public:
	static ShaderProgram* compileShader(const char* vertexShaderPath, const char* fragmentShaderPath);

private:
	static const char* shaderHeader;
	static const char* mainFunction;
};