#include "Game/Rendering/ShaderBootstrapper.hpp"

#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Time/Time.hpp"

extern int WINDOW_PHYSICAL_WIDTH;
extern int WINDOW_PHYSICAL_HEIGHT;

const char* ShaderBootstrapper::shaderHeader =
"#version 410 core\n"
"uniform vec3 iResolution;"
"uniform float iTime;"
"out vec4 outColor;\n";

const char* ShaderBootstrapper::mainFunction =
"void main()"
"{"
"    vec4 fragColor = vec4(0);"
"    vec2 fragCoord = gl_FragCoord.xy;"
"    mainImage(fragColor, fragCoord);"
"    outColor = fragColor;"
"}";

//-----------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------
void ShaderBootstrapper::initializeUniforms(ShaderProgram* program)
{
    program->SetVec3Uniform("iResolution", Vector3(static_cast<float>(WINDOW_PHYSICAL_WIDTH), static_cast<float>(WINDOW_PHYSICAL_HEIGHT), 0.5));
    program->SetFloatUniform("iTime", static_cast<float>(GetCurrentTimeSeconds()));
}

//-----------------------------------------------------------------------------------
void ShaderBootstrapper::updateUniforms(ShaderProgram* program)
{
    program->SetVec3Uniform("iResolution", Vector3(static_cast<float>(WINDOW_PHYSICAL_WIDTH), static_cast<float>(WINDOW_PHYSICAL_HEIGHT), 0.5));
    program->SetFloatUniform("iTime", static_cast<float>(GetCurrentTimeSeconds()));
}
