#pragma once

class Material;
class ShaderProgram;

//Toolkit used to bootstrap and load shaders following the ShaderToy format
class ShaderBootstrapper
{
public:
    static ShaderProgram* compileShader(const char* vertexShaderPath, const char* fragmentShaderPath);
    static void initializeUniforms(Material* material);
    static void updateUniforms(Material* material, float deltaSeconds);

private:
    static const char* shaderHeader;
    static const char* mainFunction;
};