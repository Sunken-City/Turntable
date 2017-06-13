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
class Renderable3D;
class VinylRecord;

class TheGame
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    TheGame();
    ~TheGame();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void InitializeMainCamera();
    void Update(float deltaTime);
    void CheckForImportedMeshes();
    void Render() const;
    void Begin3DPerspective() const;
    void End3DPerspective() const;
    void RenderAxisLines() const;
    void SetUpShader();
    void RenderLoadedMesh() const;
    void RenderPostProcess() const;
    void LoadDefaultScene();
    float CalculateRotationRateFromRPM(float RPM) { return (RPM * 360) / 60; }

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static TheGame* instance;
    
    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static constexpr float RPS_45 = (45.0f * 360.0f) / 60.0f;
    static constexpr float RPS_33 = (33.333333333f * 360.0f) / 60.0f;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    SoundID m_twahSFX;
    Framebuffer* m_fbo;
    VinylRecord* m_currentRecord = nullptr;

private:
    bool m_showSkeleton;
    Texture* m_pauseTexture;
    RGBA* m_color;
    Material* m_currentMaterial;
    Material* m_testMaterial;
    Material* m_uvDebugMaterial;
    Material* m_normalDebugMaterial;
    bool m_renderAxisLines;
};
