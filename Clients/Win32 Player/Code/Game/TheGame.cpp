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
#include "Engine/Renderer/3D/ForwardRenderer.hpp"
#include "Engine/Renderer/3D/Scene3D.hpp"

TheGame* TheGame::instance = nullptr;
extern MeshBuilder* g_loadedMeshBuilder;
extern Skeleton* g_loadedSkeleton;
extern AnimationMotion* g_loadedMotion;
extern std::vector<AnimationMotion*>* g_loadedMotions;
extern int g_numLoadedMeshes;

CONSOLE_COMMAND(twah)
{
    UNUSED(args);
    AudioSystem::instance->PlaySound(TheGame::instance->m_twahSFX);
}

CONSOLE_COMMAND(playsong)
{
    if (!(args.HasArgs(1) || args.HasArgs(2)))
    {
        Console::instance->PrintLine("playsound <filename> (rpm)", RGBA::RED);
        return;
    }
    std::string filepath = args.GetStringArgument(0);
    SoundID song = AudioSystem::instance->CreateOrGetSound(filepath);
    if (song == MISSING_SOUND_ID)
    {
        Console::instance->PrintLine("Could not find file.", RGBA::RED);
        return;
    }

    float frequency = 1.0f;
    if (args.HasArgs(2))
    {
        float rpm = args.GetFloatArgument(1);
        frequency = rpm / 45.0f;
        TheGame::instance->m_currentRotationRate = TheGame::instance->CalculateRotationRateFromRPM(rpm);
    }
    else
    {
        TheGame::instance->m_currentRotationRate = TheGame::RPS_45;
    }

    AudioChannelHandle channel = AudioSystem::instance->GetChannel(TheGame::instance->m_currentlyPlayingSong);
    if (AudioSystem::instance->IsPlaying(channel))
    {
        AudioSystem::instance->StopChannel(channel);
    }
    TheGame::instance->m_currentlyPlayingSong = song;
    AudioSystem::instance->PlaySound(song);
    AudioSystem::instance->MultiplyCurrentFrequency(song, frequency);
}

CONSOLE_COMMAND(stopsong)
{
    AudioChannelHandle channel = AudioSystem::instance->GetChannel(TheGame::instance->m_currentlyPlayingSong);
    if (!channel || !AudioSystem::instance->IsPlaying(channel))
    {
        Console::instance->PrintLine("No song is currently playing. Play a song using playsong first.", RGBA::RED);
        return;
    }
    else
    {
        Console::instance->PrintLine("Stopping the music. Party's over, people. :c", RGBA::GBLIGHTGREEN);
        AudioSystem::instance->StopChannel(channel);
        TheGame::instance->m_currentRotationRate = 0;
    }
}

CONSOLE_COMMAND(setsongrpm)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("setsongrpm <rpm>", RGBA::RED);
        return;
    }

    AudioChannelHandle channel = AudioSystem::instance->GetChannel(TheGame::instance->m_currentlyPlayingSong);
    if (!channel || !AudioSystem::instance->IsPlaying(channel))
    {
        Console::instance->PrintLine("No song is currently playing. Play a song using playsong first.", RGBA::RED);
        return;
    }

    float rpm = args.GetFloatArgument(0);
    float frequency = rpm / 45.0f;
    TheGame::instance->m_currentRotationRate = TheGame::instance->CalculateRotationRateFromRPM(rpm);
    AudioSystem::instance->MultiplyCurrentFrequency(TheGame::instance->m_currentlyPlayingSong, frequency);
}

MeshRenderer* quadForFBO;
unsigned int samplerID;
unsigned int diffuseID;
unsigned int normalMapID;
MeshRenderer* loadedMesh;
Material* lightMaterial;
const int NUM_LIGHTS = 1;

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
    
    LoadDefaultScene();
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
void TheGame::Update(float deltaSeconds)
{
    UpdateVinylRotation(deltaSeconds);
    UpdateVinylJacket();

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

    m_camera->Update(deltaSeconds);
    CheckForImportedMeshes();
    quadForFBO->m_material->SetFloatUniform("gTime", (float)GetCurrentTimeSeconds());

    if (InputSystem::instance->WasKeyJustPressed('B'))
    {
        m_currentMaterial = m_testMaterial;
        m_45Vinyl->m_meshRenderer.m_material = m_testMaterial;
    }
    else if (InputSystem::instance->WasKeyJustPressed('N'))
    {
        m_currentMaterial = m_normalDebugMaterial;
        m_45Vinyl->m_meshRenderer.m_material = m_normalDebugMaterial;
    }
    else if (InputSystem::instance->WasKeyJustPressed('U'))
    {
        m_currentMaterial = m_uvDebugMaterial;
        m_45Vinyl->m_meshRenderer.m_material = m_uvDebugMaterial;
    }

/*     for (int i = 0; i < 16; i++)
//     {
//         m_lightPositions[i] = Vector3(sinf(static_cast<float>(GetCurrentTimeSeconds() + i)) * 5.0f, cosf(static_cast<float>(GetCurrentTimeSeconds() + i) / 2.0f) * 3.0f, 0.5f);
//         m_lights[i].SetPosition(m_lightPositions[i]);
//         m_currentMaterial->m_shaderProgram->SetVec3Uniform(Stringf("gLightPosition[%i]", i).c_str(), m_lightPositions[i], 16);
//     }
//     
//     if (InputSystem::instance->WasKeyJustPressed('0'))
//     {
//         m_renderAxisLines = !m_renderAxisLines;
//     }
//     if (InputSystem::instance->WasKeyJustPressed('1'))
//     {
//         MeshBuilder builder;
//         MeshBuilder::PlaneData* data = new MeshBuilder::PlaneData(Vector3::ZERO, Vector3::RIGHT, Vector3::UP);
//         builder.BuildPatch(-5.0f, 5.0f, 50, -5.0f, 5.0f, 50,
//             [](const void* userData, float x, float y)
//         {
//             MeshBuilder::PlaneData const *plane = (MeshBuilder::PlaneData const*)userData;
//             Vector3 position = plane->initialPosition
//                 + (plane->right * x)
//                 + (plane->up * y);
//             return position;
//         }
//         , data);
//         builder.CopyToMesh(loadedMesh->m_mesh, &Vertex_SkinnedPCTN::Copy, sizeof(Vertex_SkinnedPCTN), &Vertex_SkinnedPCTN::BindMeshToVAO);
//         spinFactor = 0.0f;
//     }
//     if (InputSystem::instance->WasKeyJustPressed('2'))
//     {
//         MeshBuilder builder;
//         MeshBuilder::PlaneData* data = new MeshBuilder::PlaneData(Vector3::ZERO, Vector3::RIGHT, Vector3::UP);
//         builder.BuildPatch(-5.0f, 5.0f, 50, -5.0f, 5.0f, 50,
//             [](const void* userData, float x, float y)
//             {
//                 MeshBuilder::PlaneData const *plane = (MeshBuilder::PlaneData const*)userData;
//                 Vector3 position = plane->initialPosition
//                     + (plane->right * x)
//                     + (plane->up * y);
//                 position.z = sin(x + y);
//                 return position;
//             }
//         , data);
//         builder.CopyToMesh(loadedMesh->m_mesh, &Vertex_SkinnedPCTN::Copy, sizeof(Vertex_SkinnedPCTN), &Vertex_SkinnedPCTN::BindMeshToVAO);
//         spinFactor = 0.0f;
//     }
//     if (InputSystem::instance->WasKeyJustPressed('3'))
//     {
//         MeshBuilder builder;
//         MeshBuilder::PlaneData* data = new MeshBuilder::PlaneData(Vector3::ZERO, Vector3::RIGHT, Vector3::UP);
//         builder.BuildPatch(-5.0f, 5.0f, 50, -5.0f, 5.0f, 50,
//             [](const void* userData, float x, float y)
//         {
//             MeshBuilder::PlaneData const *plane = (MeshBuilder::PlaneData const*)userData;
//             Vector3 position = plane->initialPosition
//                 + (plane->right * x)
//                 + (plane->up * y);
//             position.z = .05f * -cos(((float)GetCurrentTimeSeconds() * 4.0f) + (Vector2(x, y).CalculateMagnitude() * 100.0f));
//             return position;
//         }
//         , data);
//         builder.CopyToMesh(loadedMesh->m_mesh, &Vertex_SkinnedPCTN::Copy, sizeof(Vertex_SkinnedPCTN), &Vertex_SkinnedPCTN::BindMeshToVAO);
//         spinFactor = 0.0f;
//     }
//     if (InputSystem::instance->WasKeyJustPressed('4'))
//     {
//         FbxListScene("Data/FBX/SampleBox.fbx");
//     }
//     if (InputSystem::instance->WasKeyJustPressed('5'))
//     {
//         Console::instance->RunCommand("fbxLoad Data/FBX/unitychan.fbx");
//     }
//     if (InputSystem::instance->WasKeyJustPressed('6'))
//     {
//         Console::instance->RunCommand("fbxLoad Data/FBX/samplebox.fbx");
//     }
//     if (InputSystem::instance->WasKeyJustPressed('7'))
//     {
//         Console::instance->RunCommand("saveMesh saveFile.picomesh");
//     }
//     if (InputSystem::instance->WasKeyJustPressed('8'))
//     {
//         Console::instance->RunCommand("loadMesh saveFile.picomesh");
//     }
//     if(InputSystem::instance->WasKeyJustPressed('K'))
//     {
//         m_showSkeleton = !m_showSkeleton;
//     }
//     if (InputSystem::instance->WasKeyJustPressed('I'))
//     {
//         g_loadedMotion->m_playbackMode = AnimationMotion::CLAMP;
//     }
//     else if (InputSystem::instance->WasKeyJustPressed('L'))
//     {
//         g_loadedMotion->m_playbackMode = AnimationMotion::LOOP;
//     }
//     else if (InputSystem::instance->WasKeyJustPressed('P'))
//     {
//         g_loadedMotion->m_playbackMode = AnimationMotion::PING_PONG;
//     }
//     else if (InputSystem::instance->WasKeyJustPressed('O'))
//     {
//         g_loadedMotion->m_playbackMode = AnimationMotion::PAUSED;
//     }
*/
}

//-----------------------------------------------------------------------------------
void TheGame::CheckForImportedMeshes()
{
    if (g_loadedMeshes.size() > 0)
    {
        if (g_loadedMeshes.size() > 1)
        {
            if (loadedMesh->m_mesh)
            {
                delete loadedMesh->m_mesh;
            }
            loadedMesh->m_mesh = g_loadedMeshes.front();
            g_loadedMeshes.pop();
        }
        else
        {
            if (loadedMesh->m_mesh)
            {
                delete loadedMesh->m_mesh;
            }
            loadedMesh->m_mesh = g_loadedMeshes.front();
            g_loadedMeshes.pop();
        }
    }
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateVinylRotation(float deltaSeconds)
{
    static float currentRotationRate = m_currentRotationRate;
    
    currentRotationRate = MathUtils::Lerp(0.1f, currentRotationRate, m_currentRotationRate);
    float rotationThisFrame = currentRotationRate * deltaSeconds;
    Vector3 newRotation = m_45Vinyl->m_transform.GetWorldRotationDegrees();
    newRotation.y += rotationThisFrame;
    m_45Vinyl->m_transform.SetRotationDegrees(newRotation);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateVinylJacket()
{
    static bool jacketOn = true;
    static Vector3 desiredJacketPosition = m_45Sleeve->m_transform.GetLocalPosition();
    if (InputSystem::instance->WasKeyJustPressed('J'))
    {
        jacketOn = !jacketOn;

        desiredJacketPosition.x = jacketOn ? 0.0f : -100.0f;
    }

    Vector3 currentPosition = MathUtils::Lerp(0.1f, m_45Sleeve->m_transform.GetLocalPosition(), desiredJacketPosition);
    m_45Sleeve->m_transform.SetPosition(currentPosition);
}

//-----------------------------------------------------------------------------------
void TheGame::Render() const
{
    ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(Renderer::instance->m_viewStack);
    ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(Renderer::instance->m_projStack);
    Begin3DPerspective();
    {
        RenderLoadedMesh();
        ForwardRenderer::instance->Render();
        RenderAxisLines();
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
        new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/basicLight.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
    );

    m_uvDebugMaterial = new Material(
        new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/uvDebug.frag"),
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
    );

    m_normalDebugMaterial = new Material(
        new ShaderProgram("Data/Shaders/passNormal.vert", "Data/Shaders/justNormalDebug.frag"),
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
    );
    m_testMaterial->SetDiffuseTexture("Data/Images/marth.png");
    m_currentMaterial = m_testMaterial;
}

//-----------------------------------------------------------------------------------
void TheGame::RenderLoadedMesh() const
{
    if (!loadedMesh)
    {
        return;
    }
    Matrix4x4 view = Renderer::instance->GetView();
    Matrix4x4 proj = Renderer::instance->GetProjection();
    Matrix4x4 translation;
    Matrix4x4 rotation;
    Matrix4x4 model;

    Matrix4x4::MatrixMakeTranslation(&translation, Vector3(0.0f, sin((float)GetCurrentTimeSeconds()) * spinFactor, 3.0f));
    Matrix4x4::MatrixMakeRotationAroundY(&rotation, (float)GetCurrentTimeSeconds() * spinFactor);
    Matrix4x4::MatrixMultiply(&model, &rotation, &translation);

    m_currentMaterial->SetMatrices(model, view, proj);
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

//-----------------------------------------------------------------------------------
void TheGame::LoadDefaultScene()
{
    Mesh* inner45 = MeshBuilder::LoadMesh("data/fbx/vinyl/45rpm_1.picomesh");
    Mesh* outer45 = MeshBuilder::LoadMesh("data/fbx/vinyl/45rpm_0.picomesh");
    Mesh* sleeve45 = MeshBuilder::LoadMesh("data/fbx/vinyl/45sleeve_0.picomesh");

    Material* inner45Material = new Material(
        new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/fixedVertexFormat.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
        );
    Material* outer45Material = new Material(
        new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/fixedVertexFormat.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
        );
    Material* sleeve45Material = new Material(
        new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/fixedVertexFormat.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
        );

    //inner45Material->SetDiffuseTexture("Data/Images/LabelTextures/45RPMLabel.tga");
    inner45Material->SetDiffuseTexture("Data/Images/marth.png");
    //inner45Material->SetDiffuseTexture("Data/Images/AlbumArt/p5.jpg");
    outer45Material->SetDiffuseTexture("Data/Images/DiscTextures/45RPMBaseColor.png");
    sleeve45Material->SetDiffuseTexture("Data/Images/SleeveTextures/Generic45Sleeve.tga");

    Renderable3D* inner45Renderable = new Renderable3D(inner45, inner45Material);
    m_45Sleeve = new Renderable3D(sleeve45, sleeve45Material);
    m_45Vinyl = new Renderable3D(outer45, outer45Material);

    m_45Vinyl->m_transform.AddChild(&inner45Renderable->m_transform);
    m_45Vinyl->m_transform.AddChild(&m_45Sleeve->m_transform);
    m_45Vinyl->m_transform.SetPosition(Vector3(30.0f, 0.0f, 30.0f));

    m_45Sleeve->m_transform.IgnoreParentRotation();
    m_45Sleeve->m_transform.SetRotationDegrees(Vector3(90.0f, 180.0f, 0.0f));
    m_45Sleeve->m_transform.SetScale(Vector3(1.0f, 1.0f, 0.75f));
    m_45Sleeve->m_transform.SetPosition(Vector3(0.0f, 0.4f, 0.0f));

    ForwardRenderer::instance->GetMainScene()->RegisterRenderable(inner45Renderable);
    ForwardRenderer::instance->GetMainScene()->RegisterRenderable(m_45Vinyl);
    ForwardRenderer::instance->GetMainScene()->RegisterRenderable(m_45Sleeve);
}
