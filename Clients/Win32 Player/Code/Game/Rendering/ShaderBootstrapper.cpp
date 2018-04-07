#include "Game/Rendering/ShaderBootstrapper.hpp"

#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Input/InputOutputUtils.hpp"

const char* mainFunction = "";

ShaderProgram* ShaderBootstrapper::compileShader(const char* vertexShader, const char* fragmentShader)
{
	char* vertexBuffer = FileReadIntoNewBuffer(vertexShader);
	char* fragmentBuffer = FileReadIntoNewBuffer(fragmentShader);
	ShaderProgram* shader = ShaderProgram::CreateFromShaderStrings(vertexBuffer, fragmentBuffer);

	delete vertexBuffer;
	delete fragmentBuffer;
	return shader;
}
