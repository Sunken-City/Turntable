#pragma once
#include "Engine/Audio/Audio.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/Light.hpp"

class Framebuffer;
class Texture;
class RGBA;
class Camera3D;
class Material;

class TheGame
{
public:
    TheGame();
    ~TheGame();

    void Update(float deltaTime);
    void Render() const;
    void Begin3DPerspective() const;
    void End3DPerspective() const;
    void RenderAxisLines() const;
    void SetUpShader();
    void RenderCoolStuff() const;
    void RenderPostProcess() const;
    static TheGame* instance;

    SoundID m_twahSFX;
    Framebuffer* m_fbo; 
private:
    bool m_showSkeleton;
    Texture* m_pauseTexture;
    RGBA* m_color;
    Camera3D* m_camera;
    Material* m_currentMaterial;
    Material* m_testMaterial;
    Material* m_uvDebugMaterial;
    Material* m_normalDebugMaterial;
    Material* m_pointLightMaterial;
    Vector3 m_lightPositions[16];
    Vector4 m_lightColors[16];
    Vector3 m_lightDirections[16];
    float m_lightDirectionFactor[16];
    float m_nearPower[16];
    float m_farPower[16];
    float m_nearDistance[16];
    float m_farDistance[16];
    float m_innerPower[16];
    float m_outerPower[16];
    float m_innerAngle[16];
    float m_outerAngle[16];
    Light m_lights[16];
    bool m_renderAxisLines;
};
