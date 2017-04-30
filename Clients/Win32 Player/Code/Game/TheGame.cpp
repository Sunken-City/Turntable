#include "Game/TheGame.hpp"
#include "Game/Camera3D.hpp"
#include "Game/TheApp.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Skeleton.hpp"
#include "Engine/Renderer/AnimationMotion.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Tools/fbx.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Engine/Input/InputDevices/MouseInputDevice.hpp"

TheGame* TheGame::instance = nullptr;
extern MeshBuilder* g_loadedMeshBuilder;
extern Skeleton* g_loadedSkeleton;
extern AnimationMotion* g_loadedMotion;
extern std::vector<AnimationMotion*>* g_loadedMotions;

CONSOLE_COMMAND(twah)
{
    UNUSED(args);
    AudioSystem::instance->PlaySound(TheGame::instance->m_twahSFX);
}

MeshRenderer* quadForFBO;
unsigned int samplerID;
unsigned int diffuseID;
unsigned int normalMapID;
MeshRenderer* loadedMesh;
Material* lightMaterial;
const int NUM_LIGHTS = 2;

TheGame::TheGame()
: m_pauseTexture(Texture::CreateOrGetTexture("Data/Images/Test.png"))
, m_camera(new Camera3D())
, m_twahSFX(AudioSystem::instance->CreateOrGetSound("Data/SFX/Twah.wav"))
, m_renderAxisLines(false)
, m_showSkeleton(false)
{
    SetUpShader();
#pragma TODO("Fix this blatant memory leak")
    Texture* blankTex = new Texture(1600, 900, Texture::TextureFormat::RGBA8);
    Texture* depthTex = new Texture(1600, 900, Texture::TextureFormat::D24S8);
    m_fbo = Framebuffer::FramebufferCreate(1, &blankTex, depthTex);


    Material* fboMaterial = new Material(new ShaderProgram("Data/Shaders/Post/post.vert", "Data/Shaders/Post/post.frag"), //post_pixelation
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));
    fboMaterial->SetDiffuseTexture(blankTex);
    fboMaterial->SetNormalTexture(depthTex);

    MeshBuilder builder;
    builder.AddQuad(Vector3(-1, -1, 0), Vector3::UP, 2.0f, Vector3::RIGHT, 2.0f);
    quadForFBO = new MeshRenderer(new Mesh(), fboMaterial);
    builder.CopyToMesh(quadForFBO->m_mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);

    quadForFBO->m_material->SetFloatUniform("gPixelationFactor", 8.0f);
}

TheGame::~TheGame()
{
// 	delete m_shaderProgram;
// 	glDeleteVertexArrays(1, &gVAO);
// 	glDeleteBuffers(1, &gVBO);
}
float spinFactor = 1.f;
static float animTime = 0.0f;

//-----------------------------------------------------------------------------------
void TheGame::Update(float deltaTime)
{
    DebugRenderer::instance->Update(deltaTime);
    m_camera->Update(deltaTime);

    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::TILDE))
    {
        Console::instance->ActivateConsole();
    }

    if (Console::instance->IsActive())
    {
        return; //Don't do anything involving input updates.
    }
    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ESC))
    {
        g_isQuitting = true;
        return;
    }

    for (int i = 0; i < 16; i++)
    {
        m_lightPositions[i] = Vector3(sinf(static_cast<float>(GetCurrentTimeSeconds() + i)) * 5.0f, cosf(static_cast<float>(GetCurrentTimeSeconds() + i) / 2.0f) * 3.0f, 0.5f);
        m_lights[i].SetPosition(m_lightPositions[i]);
        m_currentMaterial->m_shaderProgram->SetVec3Uniform(Stringf("gLightPosition[%i]", i).c_str(), m_lightPositions[i], 16);
    }
    
    if (InputSystem::instance->WasKeyJustPressed('0'))
    {
        m_renderAxisLines = !m_renderAxisLines;
    }
    if (InputSystem::instance->WasKeyJustPressed('1'))
    {
        MeshBuilder builder;
        MeshBuilder::PlaneData* data = new MeshBuilder::PlaneData(Vector3::ZERO, Vector3::RIGHT, Vector3::UP);
        builder.BuildPatch(-5.0f, 5.0f, 50, -5.0f, 5.0f, 50,
            [](const void* userData, float x, float y)
        {
            MeshBuilder::PlaneData const *plane = (MeshBuilder::PlaneData const*)userData;
            Vector3 position = plane->initialPosition
                + (plane->right * x)
                + (plane->up * y);
            return position;
        }
        , data);
        builder.CopyToMesh(loadedMesh->m_mesh, &Vertex_SkinnedPCTN::Copy, sizeof(Vertex_SkinnedPCTN), &Vertex_SkinnedPCTN::BindMeshToVAO);
        spinFactor = 0.0f;
    }
    if (InputSystem::instance->WasKeyJustPressed('2'))
    {
        MeshBuilder builder;
        MeshBuilder::PlaneData* data = new MeshBuilder::PlaneData(Vector3::ZERO, Vector3::RIGHT, Vector3::UP);
        builder.BuildPatch(-5.0f, 5.0f, 50, -5.0f, 5.0f, 50,
            [](const void* userData, float x, float y)
            {
                MeshBuilder::PlaneData const *plane = (MeshBuilder::PlaneData const*)userData;
                Vector3 position = plane->initialPosition
                    + (plane->right * x)
                    + (plane->up * y);
                position.z = sin(x + y);
                return position;
            }
        , data);
        builder.CopyToMesh(loadedMesh->m_mesh, &Vertex_SkinnedPCTN::Copy, sizeof(Vertex_SkinnedPCTN), &Vertex_SkinnedPCTN::BindMeshToVAO);
        spinFactor = 0.0f;
    }
    if (InputSystem::instance->WasKeyJustPressed('3'))
    {
        MeshBuilder builder;
        MeshBuilder::PlaneData* data = new MeshBuilder::PlaneData(Vector3::ZERO, Vector3::RIGHT, Vector3::UP);
        builder.BuildPatch(-5.0f, 5.0f, 50, -5.0f, 5.0f, 50,
            [](const void* userData, float x, float y)
        {
            MeshBuilder::PlaneData const *plane = (MeshBuilder::PlaneData const*)userData;
            Vector3 position = plane->initialPosition
                + (plane->right * x)
                + (plane->up * y);
            position.z = .05f * -cos(((float)GetCurrentTimeSeconds() * 4.0f) + (Vector2(x, y).CalculateMagnitude() * 100.0f));
            return position;
        }
        , data);
        builder.CopyToMesh(loadedMesh->m_mesh, &Vertex_SkinnedPCTN::Copy, sizeof(Vertex_SkinnedPCTN), &Vertex_SkinnedPCTN::BindMeshToVAO);
        spinFactor = 0.0f;
    }
    if (InputSystem::instance->WasKeyJustPressed('4'))
    {
        FbxListScene("Data/FBX/SampleBox.fbx");
    }
    if (InputSystem::instance->WasKeyJustPressed('5'))
    {
        Console::instance->RunCommand("fbxLoad Data/FBX/unitychan.fbx");
    }
    if (InputSystem::instance->WasKeyJustPressed('6'))
    {
        Console::instance->RunCommand("fbxLoad Data/FBX/samplebox.fbx");
    }
    if (InputSystem::instance->WasKeyJustPressed('7'))
    {
        Console::instance->RunCommand("saveMesh saveFile.picomesh");
    }
    if (InputSystem::instance->WasKeyJustPressed('8'))
    {
        Console::instance->RunCommand("loadMesh saveFile.picomesh");
    }
    if (g_loadedMesh != nullptr)
    {
        loadedMesh->m_mesh = g_loadedMesh;
    }
    if (InputSystem::instance->WasKeyJustPressed('B'))
    {
        m_currentMaterial = m_testMaterial;
    }
    else if (InputSystem::instance->WasKeyJustPressed('N'))
    {
        m_currentMaterial = m_normalDebugMaterial;
    }
    else if (InputSystem::instance->WasKeyJustPressed('U'))
    {
        m_currentMaterial = m_uvDebugMaterial;
    }
    if(InputSystem::instance->WasKeyJustPressed('K'))
    {
        m_showSkeleton = !m_showSkeleton;
    }
    if (InputSystem::instance->WasKeyJustPressed('I'))
    {
        g_loadedMotion->m_playbackMode = AnimationMotion::CLAMP;
    }
    else if (InputSystem::instance->WasKeyJustPressed('L'))
    {
        g_loadedMotion->m_playbackMode = AnimationMotion::LOOP;
    }
    else if (InputSystem::instance->WasKeyJustPressed('P'))
    {
        g_loadedMotion->m_playbackMode = AnimationMotion::PING_PONG;
    }
    else if (InputSystem::instance->WasKeyJustPressed('O'))
    {
        g_loadedMotion->m_playbackMode = AnimationMotion::PAUSED;
    }

    quadForFBO->m_material->SetFloatUniform("gTime", (float)GetCurrentTimeSeconds());
}

//-----------------------------------------------------------------------------------
void TheGame::Render() const
{
    ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(Renderer::instance->m_viewStack);
    ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(Renderer::instance->m_projStack);
    Begin3DPerspective();
    RenderCoolStuff();
    RenderAxisLines();
    if (g_loadedSkeleton && m_showSkeleton)
    {
        if (g_loadedMotions)
        {
            BoneMask upperHalfMask = BoneMask(g_loadedSkeleton->GetJointCount());
            upperHalfMask.SetAllBonesTo(1.0f);
            for (int i = 0; i < 9; ++i)
            {
                upperHalfMask.boneMasks[i] = 0.0f;
            }
            BoneMask lowerHalfMask = BoneMask(g_loadedSkeleton->GetJointCount());
            lowerHalfMask.SetAllBonesTo(0.0f);
            for (int i = 0; i < 9; ++i)
            {
                lowerHalfMask.boneMasks[i] = 1.0f;
            }
            g_loadedMotions->at(0)->ApplyMotionToSkeleton(g_loadedSkeleton, (float)GetCurrentTimeSeconds(), upperHalfMask);
            g_loadedMotions->at(1)->ApplyMotionToSkeleton(g_loadedSkeleton, (float)GetCurrentTimeSeconds(), lowerHalfMask);
            if (g_loadedSkeleton->m_joints)
            {
                delete g_loadedSkeleton->m_joints->m_mesh;
                delete g_loadedSkeleton->m_joints->m_material;
                delete g_loadedSkeleton->m_joints;
                g_loadedSkeleton->m_joints = nullptr;
            }
            if (g_loadedSkeleton->m_bones)
            {
                delete g_loadedSkeleton->m_bones->m_mesh;
                delete g_loadedSkeleton->m_bones->m_material;
                delete g_loadedSkeleton->m_bones;
                g_loadedSkeleton->m_bones = nullptr;
            }
        }
        g_loadedSkeleton->Render();
    }
    End3DPerspective();
    Console::instance->Render();
}

//-----------------------------------------------------------------------------------
void TheGame::Begin3DPerspective() const
{
    const float aspect = 16.f / 9.f;
    const float nearDist = 0.1f;
    const float farDist = 1000.0f;
    const float fovY = 50.0f;
    Renderer::instance->BeginPerspective(fovY, aspect, nearDist, farDist);	
    Renderer::instance->PushView(m_camera->GetViewMatrix());
}

//-----------------------------------------------------------------------------------
void TheGame::End3DPerspective() const
{
    //Reset Perspective
    Renderer::instance->EndPerspective();
    Renderer::instance->PopView();
}

//-----------------------------------------------------------------------------------
void TheGame::RenderAxisLines() const
{
    const float axisLineLength = 100.0f;
    Renderer::instance->EnableDepthTest(true);

    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(axisLineLength, 0.0f, 0.0f), RGBA::RED, 3.0f);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, axisLineLength, 0.0f), RGBA::GREEN, 3.0f);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, axisLineLength), RGBA::BLUE, 3.0f);

    Renderer::instance->EnableDepthTest(false);

    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(axisLineLength, 0.0f, 0.0f), RGBA::RED, 1.0f);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, axisLineLength, 0.0f), RGBA::GREEN, 1.0f);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, axisLineLength), RGBA::BLUE, 1.0f);

    Renderer::instance->EnableDepthTest(true);
}

//-----------------------------------------------------------------------------------
void TheGame::SetUpShader()
{
    m_testMaterial = new Material(
        new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
    );

    m_uvDebugMaterial = new Material(
        new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/uvDebug.frag"),
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
    );

    m_normalDebugMaterial = new Material(
        new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/normalDebug.frag"),
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
    );
    m_testMaterial->SetDiffuseTexture("Data/Textures/Big_Vinyl_Sleeve/BV_Example_Sleeve.tga");
    m_testMaterial->SetNormalTexture("Data/Images/stone_normal.png");
    m_testMaterial->SetEmissiveTexture("Data/Images/pattern_81/maymay.tga");
    m_testMaterial->SetNoiseTexture("Data/Images/perlinNoise.png");
    m_testMaterial->SetVec4Uniform("gDissolveColor", Vector4(0.0f, 1.0f, 0.3f, 1.0f));
    m_testMaterial->SetVec4Uniform("gColor", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    m_testMaterial->SetVec4Uniform("gAmbientLight", Vector4(1.0f, 1.0f, 1.0f, 0.0f));
    m_testMaterial->SetFloatUniform("gSpecularPower", 16.0f);
    m_testMaterial->SetFloatUniform("gSpecularIntensity", 0.5f); //0 to 1
    m_testMaterial->SetVec4Uniform("gFogColor", Vector4(0.7f, 0.7f, 0.7f, 1.0f));
    m_testMaterial->SetFloatUniform("gMinFogDistance", 10.0f);
    m_testMaterial->SetFloatUniform("gMaxFogDistance", 20.0f);
    m_testMaterial->SetIntUniform("gLightCount", NUM_LIGHTS);

    lightMaterial = new Material(
        new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag"), //fixedVertexFormat timeBased basicLight
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
        );
    lightMaterial->SetDiffuseTexture(Renderer::instance->m_defaultTexture);

    //Set all attributes of the arrays to default values
    for (int i = 0; i < 16; i++)
    {
        m_lightPositions[i] = Vector3::ZERO;
        m_lightDirections[i] = Vector3::FORWARD;
        m_lightDirectionFactor[i] = 0.0f;
        m_nearPower[i] = 1.0f;
        m_farPower[i] = 1.0f;
        m_nearDistance[i] = 2.0f;
        m_farDistance[i] = 6.0f;
        m_innerPower[i] = 1.0f;
        m_outerPower[i] = 1.0f;
        m_innerAngle[i] = 1.0f;
        m_outerAngle[i] = -1.0f;
        m_lightColors[i] = RGBA::BLACK.ToVec4(); //i % 2 == 0 ? RGBA::RED.ToVec4() : RGBA::BLUE.ToVec4();// 
    }


    //Initialize the lights for the demo
// 	m_lights[0] = Light(Vector3::ZERO, RGBA(RGBA::RED), lightMaterial);
// 	m_lights[0].ConvertToLocalPointLight(2.0f, 6.0f, 1.0f, 0.0f);
// 	m_lights[1] = Light(Vector3::ZERO, RGBA(RGBA::GREEN), lightMaterial);
// 	m_lights[1].ConvertToGlobalDirectLight(Vector3::FORWARD, 2.0f, 6.0f);

    //Initialize the arrays with our values
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        m_lightColors[i] = m_lights[i].GetColor();
        m_lightDirections[i] = m_lights[i].GetDirection();
        m_lightDirectionFactor[i] = m_lights[i].IsDirectional() ? 1.0f : 0.0f;
        m_nearPower[i] = m_lights[i].GetNearPower();
        m_farPower[i] = m_lights[i].GetFarPower();
        m_nearDistance[i] = m_lights[i].GetNearDistance();
        m_farDistance[i] = m_lights[i].GetFarDistance();
        m_innerPower[i] = m_lights[i].GetInnerPower();
        m_outerPower[i] = m_lights[i].GetOuterPower();
        m_innerAngle[i] = m_lights[i].GetInnerAngle();
        m_outerAngle[i] = m_lights[i].GetOuterAngle();
    }

    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        m_testMaterial->SetVec4Uniform(Stringf("gLightColor[%i]", i).c_str(), m_lightColors[i], 16);
        m_testMaterial->SetVec3Uniform(Stringf("gLightDirection[%i]", i).c_str(), m_lightDirections[i], 16);
        m_testMaterial->SetFloatUniform(Stringf("gLightDirectionFactor[%i]", i).c_str(), m_lightDirectionFactor[i], 16);
        m_testMaterial->SetFloatUniform(Stringf("gNearPower[%i]", i).c_str(), m_nearPower[i], 16);
        m_testMaterial->SetFloatUniform(Stringf("gFarPower[%i]", i).c_str(), m_farPower[i], 16);
        m_testMaterial->SetFloatUniform(Stringf("gNearDistance[%i]", i).c_str(), m_nearDistance[i], 16);
        m_testMaterial->SetFloatUniform(Stringf("gFarDistance[%i]", i).c_str(), m_farDistance[i], 16);
        m_testMaterial->SetFloatUniform(Stringf("gInnerPower[%i]", i).c_str(), m_innerPower[i], 16);
        m_testMaterial->SetFloatUniform(Stringf("gOuterPower[%i]", i).c_str(), m_outerPower[i], 16);
        m_testMaterial->SetFloatUniform(Stringf("gInnerAngle[%i]", i).c_str(), m_innerAngle[i], 16);
        m_testMaterial->SetFloatUniform(Stringf("gOuterAngle[%i]", i).c_str(), m_outerAngle[i], 16);
    }
    m_currentMaterial = m_testMaterial;

    MeshBuilder builder;
    builder.AddCube(2.0f);
    //Lol more blatant memory leaks fml
    loadedMesh = new MeshRenderer(new Mesh(), m_currentMaterial);
    builder.CopyToMesh(loadedMesh->m_mesh, &Vertex_SkinnedPCTN::Copy, sizeof(Vertex_SkinnedPCTN), &Vertex_SkinnedPCTN::BindMeshToVAO);
    

    m_uvDebugMaterial->SetDiffuseTexture(Renderer::instance->m_defaultTexture);
    m_uvDebugMaterial->SetNormalTexture(Renderer::instance->m_defaultTexture);
    m_uvDebugMaterial->SetEmissiveTexture("Data/Images/pattern_81/maymay.tga");
    m_uvDebugMaterial->SetNoiseTexture("Data/Images/perlinNoise.png");
    m_uvDebugMaterial->SetVec4Uniform("gDissolveColor", Vector4(0.0f, 1.0f, 0.3f, 1.0f));
    m_uvDebugMaterial->SetVec4Uniform("gColor", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    m_uvDebugMaterial->SetVec4Uniform("gAmbientLight", Vector4(1.0f, 1.0f, 1.0f, 0.0f));
    m_uvDebugMaterial->SetFloatUniform("gSpecularPower", 16.0f);
    m_uvDebugMaterial->SetFloatUniform("gSpecularIntensity", 0.5f); //0 to 1
    m_uvDebugMaterial->SetVec4Uniform("gFogColor", Vector4(0.7f, 0.7f, 0.7f, 1.0f));
    m_uvDebugMaterial->SetFloatUniform("gMinFogDistance", 10.0f);
    m_uvDebugMaterial->SetFloatUniform("gMaxFogDistance", 20.0f);
    m_uvDebugMaterial->SetIntUniform("gLightCount", NUM_LIGHTS);
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        m_uvDebugMaterial->SetVec4Uniform(Stringf("gLightColor[%i]", i).c_str(), m_lightColors[i], 16);
        m_uvDebugMaterial->SetVec3Uniform(Stringf("gLightDirection[%i]", i).c_str(), m_lightDirections[i], 16);
        m_uvDebugMaterial->SetFloatUniform(Stringf("gLightDirectionFactor[%i]", i).c_str(), m_lightDirectionFactor[i], 16);
        m_uvDebugMaterial->SetFloatUniform(Stringf("gNearPower[%i]", i).c_str(), m_nearPower[i], 16);
        m_uvDebugMaterial->SetFloatUniform(Stringf("gFarPower[%i]", i).c_str(), m_farPower[i], 16);
        m_uvDebugMaterial->SetFloatUniform(Stringf("gNearDistance[%i]", i).c_str(), m_nearDistance[i], 16);
        m_uvDebugMaterial->SetFloatUniform(Stringf("gFarDistance[%i]", i).c_str(), m_farDistance[i], 16);
        m_uvDebugMaterial->SetFloatUniform(Stringf("gInnerPower[%i]", i).c_str(), m_innerPower[i], 16);
        m_uvDebugMaterial->SetFloatUniform(Stringf("gOuterPower[%i]", i).c_str(), m_outerPower[i], 16);
        m_uvDebugMaterial->SetFloatUniform(Stringf("gInnerAngle[%i]", i).c_str(), m_innerAngle[i], 16);
        m_uvDebugMaterial->SetFloatUniform(Stringf("gOuterAngle[%i]", i).c_str(), m_outerAngle[i], 16);
    }

    m_normalDebugMaterial->SetDiffuseTexture("Data/Images/stone_diffuse.png");
    m_normalDebugMaterial->SetNormalTexture("Data/Images/stone_normal.png");
    m_normalDebugMaterial->SetEmissiveTexture("Data/Images/pattern_81/maymay.tga");
    m_normalDebugMaterial->SetNoiseTexture("Data/Images/perlinNoise.png");
    m_normalDebugMaterial->SetVec4Uniform("gDissolveColor", Vector4(0.0f, 1.0f, 0.3f, 1.0f));
    m_normalDebugMaterial->SetVec4Uniform("gColor", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    m_normalDebugMaterial->SetVec4Uniform("gAmbientLight", Vector4(1.0f, 1.0f, 1.0f, 0.0f));
    m_normalDebugMaterial->SetFloatUniform("gSpecularPower", 16.0f);
    m_normalDebugMaterial->SetFloatUniform("gSpecularIntensity", 0.5f); //0 to 1
    m_normalDebugMaterial->SetVec4Uniform("gFogColor", Vector4(0.7f, 0.7f, 0.7f, 1.0f));
    m_normalDebugMaterial->SetFloatUniform("gMinFogDistance", 10.0f);
    m_normalDebugMaterial->SetFloatUniform("gMaxFogDistance", 20.0f);
    m_normalDebugMaterial->SetIntUniform("gLightCount", NUM_LIGHTS);
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        m_normalDebugMaterial->SetVec4Uniform(Stringf("gLightColor[%i]", i).c_str(), m_lightColors[i], 16);
        m_normalDebugMaterial->SetVec3Uniform(Stringf("gLightDirection[%i]", i).c_str(), m_lightDirections[i], 16);
        m_normalDebugMaterial->SetFloatUniform(Stringf("gLightDirectionFactor[%i]", i).c_str(), m_lightDirectionFactor[i], 16);
        m_normalDebugMaterial->SetFloatUniform(Stringf("gNearPower[%i]", i).c_str(), m_nearPower[i], 16);
        m_normalDebugMaterial->SetFloatUniform(Stringf("gFarPower[%i]", i).c_str(), m_farPower[i], 16);
        m_normalDebugMaterial->SetFloatUniform(Stringf("gNearDistance[%i]", i).c_str(), m_nearDistance[i], 16);
        m_normalDebugMaterial->SetFloatUniform(Stringf("gFarDistance[%i]", i).c_str(), m_farDistance[i], 16);
        m_normalDebugMaterial->SetFloatUniform(Stringf("gInnerPower[%i]", i).c_str(), m_innerPower[i], 16);
        m_normalDebugMaterial->SetFloatUniform(Stringf("gOuterPower[%i]", i).c_str(), m_outerPower[i], 16);
        m_normalDebugMaterial->SetFloatUniform(Stringf("gInnerAngle[%i]", i).c_str(), m_innerAngle[i], 16);
        m_normalDebugMaterial->SetFloatUniform(Stringf("gOuterAngle[%i]", i).c_str(), m_outerAngle[i], 16);
    }

    int NUM_BONES = 200;
    Matrix4x4 mat = Matrix4x4::IDENTITY;
    for (int i = 0; i < NUM_BONES; ++i)
    {
        int x = 4;
        x++;
        //Bones are suspended lmao
        //m_testMaterial->SetMatrix4x4Uniform(Stringf("gBoneMatrices[%i]", i).c_str(), mat, NUM_BONES);
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderCoolStuff() const
{
    Matrix4x4 view = Renderer::instance->GetView();
    Matrix4x4 proj = Renderer::instance->GetProjection();
    Matrix4x4 translation;
    Matrix4x4 rotation;
    Matrix4x4 model;

    m_currentMaterial->m_shaderProgram->SetVec3Uniform("gCameraPosition", m_camera->m_position);

    Matrix4x4::MatrixMakeTranslation(&translation, Vector3(0.0f, sin((float)GetCurrentTimeSeconds()) * spinFactor, 3.0f));
    Matrix4x4::MatrixMakeRotationAroundY(&rotation, (float)GetCurrentTimeSeconds() * spinFactor);
    Matrix4x4::MatrixMultiply(&model, &rotation, &translation);

    if (g_loadedMotion && g_loadedSkeleton)
    {
        int NUM_BONES = 200;
        for (int i = 0; i < g_loadedSkeleton->m_modelToBoneSpace.size(); ++i)
        {
            Matrix4x4 inverseWorld = g_loadedSkeleton->m_modelToBoneSpace[i];
            Matrix4x4 mat = Matrix4x4::IDENTITY;
            Matrix4x4::MatrixMultiply(&mat, &inverseWorld, &g_loadedSkeleton->m_boneToModelSpace[i]);
            m_testMaterial->SetMatrix4x4Uniform(Stringf("gBoneMatrices[%i]", i).c_str(), mat, NUM_BONES);
        }
    }

    m_currentMaterial->SetMatrices(model, view, proj);
    GL_CHECK_ERROR();
    loadedMesh->m_material = m_currentMaterial;
    loadedMesh->Render();
}

//-----------------------------------------------------------------------------------
void TheGame::RenderPostProcess() const
{
    Matrix4x4 model;
    Matrix4x4 view;
    Matrix4x4 proj;
    Matrix4x4::MatrixMakeIdentity(&model);
    Matrix4x4::MatrixMakeIdentity(&view);
    Matrix4x4::MatrixMakeIdentity(&proj);
    quadForFBO->m_material->SetMatrices(model, view, proj);
    quadForFBO->m_material->BindAvailableTextures();
    quadForFBO->Render();
}
