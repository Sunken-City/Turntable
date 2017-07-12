#include "Game/TheGame.hpp"
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
#include "Engine/Renderer/3D/Camera3D.hpp"
#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "ThirdParty/taglib/include/taglib/tag.h"
#include "ThirdParty/taglib/include/taglib/fileref.h"
#include "ThirdParty/taglib/include/taglib/tfile.h"
#include "ThirdParty/taglib/include/taglib/taglib.h"
#include "ThirdParty/taglib/include/taglib/tstring.h"
#include "ThirdParty/taglib/include/taglib/flacfile.h"
#include "ThirdParty/taglib/include/taglib/wavfile.h"
#include "ThirdParty/taglib/include/taglib/mpegfile.h"
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Engine/Input/InputDevices/MouseInputDevice.hpp"
#include "Engine/Renderer/3D/ForwardRenderer.hpp"
#include "Engine/Renderer/3D/Scene3D.hpp"
#include "Renderables/VinylRecord.hpp"
#include "Audio/SongManager.hpp"
#include "Engine/UI/UISystem.hpp"
TheGame* TheGame::instance = nullptr;
extern MeshBuilder* g_loadedMeshBuilder;
extern Skeleton* g_loadedSkeleton;
extern AnimationMotion* g_loadedMotion;
extern std::vector<AnimationMotion*>* g_loadedMotions;
extern int g_numLoadedMeshes;

MeshRenderer* quadForFBO;
unsigned int samplerID;
unsigned int diffuseID;
unsigned int normalMapID;
MeshRenderer* loadedMesh;
Material* lightMaterial;
const int NUM_LIGHTS = 1;
float spinFactor = 1.f;
static float animTime = 0.0f;

TheGame::TheGame()
: m_pauseTexture(Texture::CreateOrGetTexture("Data/Images/Test.png"))
{
    SongManager::instance = new SongManager();
    UISystem::instance->LoadAndParseUIXML("Data/UI/PlayerLayout.xml");

    SetUpShader();
    Texture* blankFBOTexture = new Texture(1600, 900, Texture::TextureFormat::RGBA8);
    Texture* depthTexture1 = new Texture(1600, 900, Texture::TextureFormat::D24S8);
    m_fbo = Framebuffer::FramebufferCreate(1, &blankFBOTexture, depthTexture1);

    m_fboMaterial = new Material(new ShaderProgram("Data/Shaders/Post/post.vert", "Data/Shaders/Backgrounds/earthbound.frag"), //post_pixelation
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));
    m_fboMaterial->SetDiffuseTexture(blankFBOTexture);
    m_fboMaterial->SetNormalTexture(depthTexture1);

    MeshBuilder builder;
    builder.AddQuad(Vector3(-1, -1, 0), Vector3::UP, 2.0f, Vector3::RIGHT, 2.0f);
    quadForFBO = new MeshRenderer(new Mesh(), m_fboMaterial);
    builder.CopyToMesh(quadForFBO->m_mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);

    quadForFBO->m_material->SetFloatUniform("gPixelationFactor", 8.0f);

    //PrintConsoleWelcome();    
    LoadDefaultScene(); 
    InitializeMainCamera();
}

//-----------------------------------------------------------------------------------
TheGame::~TheGame()
{
    delete SongManager::instance;
    SongManager::instance = nullptr;
    delete m_currentRecord;
    delete m_fbo->m_colorTargets[0];
    delete m_fbo->m_depthStencilTarget;
    delete quadForFBO->m_mesh;
    delete quadForFBO;
    delete loadedMesh;
    delete lightMaterial;
    delete m_testMaterial->m_shaderProgram;
    delete m_uvDebugMaterial->m_shaderProgram;
    delete m_normalDebugMaterial->m_shaderProgram;
    delete m_fboMaterial->m_shaderProgram;
    delete m_fboMaterial;
    delete m_testMaterial;
    delete m_uvDebugMaterial;
    delete m_normalDebugMaterial;
    Framebuffer::FramebufferDelete(m_fbo);
}

//-----------------------------------------------------------------------------------
void TheGame::PrintConsoleWelcome()
{
    //Unicode support motivation
    Console::instance->PrintLine("Welcome to", RGBA::YELLOW);
    Console::instance->PrintLine("████████╗██╗   ██╗██████╗ ███╗   ██╗████████╗ █████╗ ██████╗ ██╗     ███████╗", RGBA::YELLOW);
    Console::instance->PrintLine("╚══██╔══╝██║   ██║██╔══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔══██╗██║     ██╔════╝", RGBA::YELLOW);
    Console::instance->PrintLine("   ██║   ██║   ██║██████╔╝██╔██╗ ██║   ██║   ███████║██████╔╝██║     █████╗  ", RGBA::YELLOW);
    Console::instance->PrintLine("   ██║   ██║   ██║██╔══██╗██║╚██╗██║   ██║   ██╔══██║██╔══██╗██║     ██╔══╝  ", RGBA::YELLOW);
    Console::instance->PrintLine("   ██║   ╚██████╔╝██║  ██║██║ ╚████║   ██║   ██║  ██║██████╔╝███████╗███████╗", RGBA::YELLOW);
    Console::instance->PrintLine("   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝╚═════╝ ╚══════╝╚══════╝", RGBA::YELLOW);
}

//-----------------------------------------------------------------------------------
void TheGame::InitializeMainCamera()
{
    Camera3D* camera = ForwardRenderer::instance->GetMainCamera();
    camera->m_updateFromInput = false;
    camera->m_position = Vector3(30.0f, 30.0f, 0.0f);
    camera->LookAt(m_currentRecord->GetPosition());
}

//-----------------------------------------------------------------------------------
void TheGame::Update(float deltaSeconds)
{
    m_currentRecord->Update(deltaSeconds);
    SongManager::instance->Update(deltaSeconds);
    quadForFBO->m_material->SetFloatUniform("gTime", (float)GetCurrentTimeSeconds());

    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::TILDE))
    {
        Console::instance->ActivateConsole();
    }
    if (Console::instance->IsActive())
    {
        return; //Don't do anything involving input updates.
    }
    else if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ESC))
    {
        g_isQuitting = true;
        return;
    }

    if (InputSystem::instance->WasKeyJustPressed('R'))
    {
        UISystem::instance->ReloadUI("Data/UI/PlayerLayout.xml");
    }
    if (InputSystem::instance->WasKeyJustPressed('L'))
    {
        Camera3D* camera = ForwardRenderer::instance->GetMainCamera();
        camera->m_updateFromInput = !camera->m_updateFromInput;
        m_renderAxisLines = !m_renderAxisLines; //Show the axis lines when we're moving in 3D space for debugging purposes.
        if (!camera->m_updateFromInput)
        {
            MouseInputDevice::ReleaseMouseCursor();
            MouseInputDevice::ShowMouseCursor();
            camera->LookAt(m_currentRecord->GetPosition());
        }
    }

    ForwardRenderer::instance->Update(deltaSeconds);
    CheckForImportedMeshes();

    if (InputSystem::instance->WasKeyJustPressed('B'))
    {
        m_currentMaterial = m_testMaterial;
        m_currentRecord->m_vinyl->m_meshRenderer.m_material = m_testMaterial;
        m_currentRecord->m_vinylLabel->m_meshRenderer.m_material = m_testMaterial;
    }
    else if (InputSystem::instance->WasKeyJustPressed('N'))
    {
        m_currentMaterial = m_normalDebugMaterial;
        m_currentRecord->m_vinyl->m_meshRenderer.m_material = m_normalDebugMaterial;
        m_currentRecord->m_vinylLabel->m_meshRenderer.m_material = m_normalDebugMaterial;
    }
    else if (InputSystem::instance->WasKeyJustPressed('U'))
    {
        m_currentMaterial = m_uvDebugMaterial;
        m_currentRecord->m_vinyl->m_meshRenderer.m_material = m_uvDebugMaterial;
        m_currentRecord->m_vinylLabel->m_meshRenderer.m_material = m_uvDebugMaterial;
    }
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
    Renderer::instance->PushView(ForwardRenderer::instance->GetMainCamera()->GetViewMatrix());
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
    if (!m_renderAxisLines)
    {
        return;
    }
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
    m_currentRecord = new VinylRecord(VinylRecord::Type::RPM_45);
    m_currentRecord->AddToScene(ForwardRenderer::instance->GetMainScene());
}

//CONSOLE COMMANDS/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(use33)
{
    UNUSED(args);
    if (TheGame::instance->m_currentRecord->m_type == VinylRecord::RPM_33)
    {
        Console::instance->PrintLine("Already using a 33RPM record", RGBA::RED);
        return;
    }
    if (SongManager::instance->IsPlaying())
    {
        Console::instance->PrintLine("Please stop the currently playing song", RGBA::RED);
        return;
    }
    VinylRecord* record = new VinylRecord(VinylRecord::Type::RPM_33);
    TheGame::instance->m_currentRecord->RemoveFromScene(ForwardRenderer::instance->GetMainScene());
    record->AddToScene(ForwardRenderer::instance->GetMainScene());
    delete TheGame::instance->m_currentRecord;
    TheGame::instance->m_currentRecord = record;
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(use45)
{
    UNUSED(args);
    if (TheGame::instance->m_currentRecord->m_type == VinylRecord::RPM_45)
    {
        Console::instance->PrintLine("Already using a 45RPM record", RGBA::RED);
        return;
    }
    if (SongManager::instance->IsPlaying())
    {
        Console::instance->PrintLine("Please stop the currently playing song", RGBA::RED);
        return;
    }
    VinylRecord* record = new VinylRecord(VinylRecord::Type::RPM_45);
    TheGame::instance->m_currentRecord->RemoveFromScene(ForwardRenderer::instance->GetMainScene());
    record->AddToScene(ForwardRenderer::instance->GetMainScene());
    delete TheGame::instance->m_currentRecord;
    TheGame::instance->m_currentRecord = record;
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(getsongmetadata)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("getsongmetadata <filename>", RGBA::RED);
        return;
    }
    std::string filepath = args.GetStringArgument(0);
    SoundID song = AudioSystem::instance->CreateOrGetSound(filepath);
    if (song == MISSING_SOUND_ID)
    {
        //Try again with the current working directory added to the path
        std::wstring cwd = Console::instance->GetCurrentWorkingDirectory();
        filepath = std::string(cwd.begin(), cwd.end()) + "\\" + filepath;
        song = AudioSystem::instance->CreateOrGetSound(filepath);

        if (song == MISSING_SOUND_ID)
        {
            Console::instance->PrintLine("Could not find file.", RGBA::RED);
            return;
        }
    }

    //Why isn't this working?
    TagLib::FileRef audioFile(filepath.c_str());
    TagLib::String artist = audioFile.tag()->artist();
    TagLib::String album = audioFile.tag()->album();
    int year = audioFile.tag()->year();
    Console::instance->PrintLine(Stringf("Artist: %s\n", artist.toCString()));
    Console::instance->PrintLine(Stringf("Album: %s\n", album.toCString()));
    Console::instance->PrintLine(Stringf("Year: %i\n", year));
}
