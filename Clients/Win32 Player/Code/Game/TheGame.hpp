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

class TheGame
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    TheGame();
    ~TheGame();

    ////FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaTime);
    void UpdateVinylRotation(float deltaSeconds);
    void UpdateVinylJacket();
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
    static constexpr float RPS_45 = (45 * 360) / 60;
    static constexpr float RPS_33 = (33.333333333 * 360) / 60;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    SoundID m_twahSFX;
    SoundID m_currentlyPlayingSong;
    Framebuffer* m_fbo;

    Renderable3D* m_45Vinyl = nullptr;
    Renderable3D* m_45VinylLabel = nullptr;
    Renderable3D* m_45Sleeve = nullptr;
    Material* m_inner45Material;
    Material* m_outer45Material;

    float m_currentRotationRate = 0.0f;

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
