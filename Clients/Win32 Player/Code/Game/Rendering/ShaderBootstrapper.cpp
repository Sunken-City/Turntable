#include "Game/Rendering/ShaderBootstrapper.hpp"

#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Input/InputOutputUtils.hpp"

const char* ShaderBootstrapper::shaderHeader =
"#version 410 core\n"
"out vec4 outColor;\n";

const char* ShaderBootstrapper::mainFunction =
"void main()"
"{"
"    vec4 fragColor = vec4(0);"
"    vec2 fragCoord = gl_FragCoord.xy;"
"    mainImage(fragColor, fragCoord);"
"    outColor = fragColor;"
"}";

ShaderProgram* ShaderBootstrapper::compileShader(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	char* vertexBuffer = FileReadIntoNewBuffer(vertexShaderPath);
	char* fragShader = FileReadIntoNewBuffer(fragmentShaderPath);
	std::string fragmentBuffer = shaderHeader;
	fragmentBuffer.append(fragShader);
	fragmentBuffer.append(mainFunction);

	ShaderProgram* shader = ShaderProgram::CreateFromShaderStrings(vertexBuffer, fragmentBuffer.c_str());

	delete vertexBuffer;
	delete fragShader;
	return shader;
}
