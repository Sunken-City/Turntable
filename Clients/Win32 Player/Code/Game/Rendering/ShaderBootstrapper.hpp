#pragma once

class Material;
class ShaderProgram;
class Texture;

//Toolkit used to bootstrap and load shaders following the ShaderToy format
class ShaderBootstrapper
{
public:
    static ShaderProgram* compileShader(const char* vertexShaderPath, const char* fragmentShaderPath);
    static void initializeUniforms(Material* material);
    static void updateUniforms(Material* material, float deltaSeconds);

private:
    static const char* s_shaderHeader;
    static const char* s_mainFunction;
    static const Texture* s_defaultAudioTexture;
    static const Texture* s_currentAudioTexture;
};