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
    void SetUpShaders();
    void RenderLoadedMesh() const;
    void RenderPostProcess() const;
    void LoadDefaultScene();
    float CalculateRotationRateFromRPM(float RPM) { return (RPM * 360) / 60; }
    void PrintConsoleWelcome();

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static TheGame* instance;
    
    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static constexpr float RPS_45 = (45.0f * 360.0f) / 60.0f;
    static constexpr float RPS_33 = (33.333333333f * 360.0f) / 60.0f;
    static constexpr unsigned int NUM_PROC_GEN_MATERIALS = 2;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Framebuffer* m_fbo = nullptr;
    VinylRecord* m_currentRecord = nullptr;
    Material* m_fboMaterial = nullptr;
    Material* m_proceduralGenerationMaterials[NUM_PROC_GEN_MATERIALS];
    Texture* m_blankFBOColorTexture = nullptr;
    Texture* m_blankFBODepthTexture = nullptr;
    MeshRenderer* m_quadForFBO = nullptr;

private:
    Texture* m_pauseTexture = nullptr;
    Material* m_currentMaterial = nullptr;
    Material* m_testMaterial = nullptr;
    Material* m_uvDebugMaterial = nullptr;
    Material* m_normalDebugMaterial = nullptr;
    Material* m_backdropMaterial = nullptr;
    bool m_renderAxisLines = false;
};
