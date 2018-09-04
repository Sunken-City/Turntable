#include "Game/Rendering/ShaderBootstrapper.hpp"

#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Time/Time.hpp"
#include "Game/TheGame.hpp"
#include "Game/Renderables/VinylRecord.hpp"
#include "Game/Audio/SongManager.hpp"
#include "Game/Audio/Song.hpp"
#include <chrono>
#include <ctime>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Renderer/Texture.hpp"

extern int WINDOW_PHYSICAL_WIDTH;
extern int WINDOW_PHYSICAL_HEIGHT;

const char* ShaderBootstrapper::s_shaderHeader =
"#version 410 core\n"
"uniform vec3 iResolution;"
"uniform float iTime;"
"uniform float iTimeDelta;"
"uniform float iFrame;"
"uniform float iFrameRate;"
"uniform float iChannelTime[4];"
"uniform vec4 iMouse;"
"uniform vec4 iDate;"
"uniform float iSampleRate;"
"uniform vec3 iChannelResolution[4];"
"uniform sampler2D iChannel0;"
"uniform sampler2D iChannel1;"
"in vec2 passUV0;"
"out vec4 outColor;\n";

const char* ShaderBootstrapper::s_mainFunction =
"void main()"
"{"
"    vec4 fragColor = vec4(0);"
"    vec2 fragCoord = gl_FragCoord.xy;"
"    mainImage(fragColor, fragCoord);"
"    outColor = fragColor;"
"}";

const Texture* ShaderBootstrapper::s_defaultAudioTexture = nullptr;
const Texture* ShaderBootstrapper::s_currentAudioTexture = nullptr;

//-----------------------------------------------------------------------------------
ShaderProgram* ShaderBootstrapper::compileShader(const char* vertexShaderPath, const char* fragmentShaderPath)
{
    char* vertexBuffer = FileReadIntoNewBuffer(vertexShaderPath);
    char* fragShader = FileReadIntoNewBuffer(fragmentShaderPath);
    std::string fragmentBuffer = s_shaderHeader;
    fragmentBuffer.append(fragShader);
    fragmentBuffer.append(s_mainFunction);

    ShaderProgram* shader = ShaderProgram::CreateFromShaderStrings(vertexBuffer, fragmentBuffer.c_str());

    delete vertexBuffer;
    delete fragShader;
    return shader;
}

//-----------------------------------------------------------------------------------
void ShaderBootstrapper::initializeUniforms(Material* material)
{
    ShaderProgram* program = material->m_shaderProgram;
    float iChannelTime[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    Vector3 iChannelResolution[4] = { Vector3::ZERO, Vector3::ZERO, Vector3::ZERO, Vector3::ZERO };
    std::time_t timeNow = std::time(nullptr);
    //Supressed because localtime_s isn't actually accessible because I don't know why try for yourself. >:I
#pragma warning(suppress: 4996)
    std::tm time = *std::localtime(&timeNow);

    program->SetVec3Uniform("iResolution", Vector3(static_cast<float>(WINDOW_PHYSICAL_WIDTH), static_cast<float>(WINDOW_PHYSICAL_HEIGHT), 1.0f));
    program->SetFloatUniform("iTime", static_cast<float>(GetCurrentTimeSeconds()));
    program->SetFloatUniform("iTimeDelta", 0.0f);
    program->SetFloatUniform("iFrame", 0.0f);
    program->SetFloatUniform("iFrameRate", GetCurrentFrameRate());
    program->SetFloatUniform("iChannelTime", iChannelTime, 4);
    program->SetVec4Uniform("iMouse", Vector4(0.0f));
    program->SetVec4Uniform("iDate", Vector4(static_cast<float>(time.tm_year - 1), static_cast<float>(time.tm_mon - 1), static_cast<float>(time.tm_mday), static_cast<float>(time.tm_sec))); //TODO: include fractional part of sec.
    program->SetFloatUniform("iSampleRate", 0.0f);
    program->SetVec3Uniform("iChannelResolution", iChannelResolution, 4);

    float audioData[AudioSystem::SPECTRUM_SIZE * 2];
    memset(audioData, 0, AudioSystem::SPECTRUM_SIZE * 2 * sizeof(float));
    s_defaultAudioTexture = new Texture(AudioSystem::SPECTRUM_SIZE, 2, Texture::TextureFormat::R32UI, audioData);
    s_currentAudioTexture = s_defaultAudioTexture;

    if (TheGame::instance)
    {
        glActiveTexture(GL_TEXTURE0 + 4);
        glBindTexture(GL_TEXTURE_2D, TheGame::instance->m_currentRecord->m_innerMaterial->m_diffuseID);
        glBindSampler(4, material->m_samplerID);
        program->SetIntUniform("iChannel0", 4);

        if (SongManager::instance && SongManager::instance->IsPlaying())
        {
            AudioSystem::instance->GetSpectrumData(SongManager::instance->m_activeSong->m_audioChannelHandle, audioData);
            AudioSystem::instance->GetWaveData(SongManager::instance->m_activeSong->m_audioChannelHandle, audioData + AudioSystem::SPECTRUM_SIZE);
            s_currentAudioTexture = new Texture(AudioSystem::SPECTRUM_SIZE, 2, Texture::TextureFormat::R32UI, audioData);
        }

        glActiveTexture(GL_TEXTURE0 + 5);
        glBindTexture(GL_TEXTURE_2D, s_currentAudioTexture->m_openglTextureID);
        glBindSampler(5, material->m_samplerID);
        program->SetIntUniform("iChannel1", 5);
    }
}

//-----------------------------------------------------------------------------------
void ShaderBootstrapper::updateUniforms(Material* material, float deltaSeconds)
{
    ShaderProgram* program = material->m_shaderProgram;

    //Calculate mouse vector
    bool isClicking = InputSystem::instance->IsMouseButtonDown(InputSystem::LEFT_MOUSE_BUTTON);
    Vector2Int lastClickedPos = InputSystem::instance->GetMouseLastClickedPos();
    Vector2 currentPos = isClicking ? Vector2(InputSystem::instance->GetMousePos()) : Vector2(lastClickedPos);
    Vector2 dragPos = isClicking ? Vector2(lastClickedPos) : Vector2(-lastClickedPos);
    //Change the origin from top-left to bottom-left
    currentPos.y = static_cast<float>(WINDOW_PHYSICAL_HEIGHT) - currentPos.y;
    dragPos.y = isClicking ? static_cast<float>(WINDOW_PHYSICAL_HEIGHT) - dragPos.y : -(static_cast<float>(WINDOW_PHYSICAL_HEIGHT) - (-dragPos.y));

    //Calculate song channel time
    float songTime = 0.0f;
    float sampleRate = 0.0f;
    if (SongManager::instance->IsPlaying())
    {
        songTime = static_cast<float>(AudioSystem::instance->GetPlaybackPositionMS(SongManager::instance->m_activeSong->m_audioChannelHandle)) / 1000.0f;
        sampleRate = static_cast<float>(SongManager::instance->m_activeSong->m_samplerate);
    }
    float iChannelTime[4] = { 0.0f, songTime, 0.0f, 0.0f };

    //Calculate date
    std::time_t timeNow = std::time(nullptr);
    //Supressed because localtime_s isn't actually accessible because I don't know why try for yourself. >:I
#pragma warning(suppress: 4996)
    std::tm time = *std::localtime(&timeNow);

    program->SetFloatUniform("iTime", static_cast<float>(GetCurrentTimeSeconds()));
    program->SetFloatUniform("iTimeDelta", deltaSeconds);
    program->SetFloatUniform("iFrame", static_cast<float>(GetFrameNumber() - program->m_frameCreated));
    program->SetFloatUniform("iFrameRate", GetCurrentFrameRate());
    program->SetFloatUniform("iChannelTime", iChannelTime, 4);
    program->SetVec4Uniform("iMouse", Vector4(currentPos, dragPos));
    program->SetVec4Uniform("iDate", Vector4(static_cast<float>(time.tm_year - 1), static_cast<float>(time.tm_mon - 1), static_cast<float>(time.tm_mday), static_cast<float>(time.tm_sec))); //TODO: include fractional part of sec.
    program->SetFloatUniform("iSampleRate", sampleRate);

    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, TheGame::instance->m_currentRecord->m_innerMaterial->m_diffuseID);
    glBindSampler(4, material->m_samplerID);
    program->SetIntUniform("iChannel0", 4);

    float audioData[AudioSystem::SPECTRUM_SIZE * 2];
    memset(audioData, 0, AudioSystem::SPECTRUM_SIZE * 2 * sizeof(float));
    if (SongManager::instance && SongManager::instance->IsPlaying())
    {
        if (s_currentAudioTexture && s_currentAudioTexture != s_defaultAudioTexture)
        {
            delete s_currentAudioTexture;
        }
        AudioSystem::instance->GetSpectrumData(SongManager::instance->m_activeSong->m_audioChannelHandle, audioData);
        AudioSystem::instance->GetWaveData(SongManager::instance->m_activeSong->m_audioChannelHandle, audioData + AudioSystem::SPECTRUM_SIZE);
        s_currentAudioTexture = new Texture(AudioSystem::SPECTRUM_SIZE, 2, Texture::TextureFormat::R32UI, audioData);
    }
    else
    {
        s_currentAudioTexture = s_defaultAudioTexture;
    }

    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_2D, s_currentAudioTexture->m_openglTextureID);
    glBindSampler(5, material->m_samplerID);
    program->SetIntUniform("iChannel1", 5);
    
}
