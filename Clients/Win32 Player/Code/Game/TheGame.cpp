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
#include "Engine/Input/InputDevices/MouseInputDevice.hpp"
#include "Engine/Renderer/3D/ForwardRenderer.hpp"
#include "Engine/Renderer/3D/Scene3D.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "ThirdParty/taglib/include/taglib/tag.h"
#include "ThirdParty/taglib/include/taglib/fileref.h"
#include "ThirdParty/taglib/include/taglib/tfile.h"
#include "ThirdParty/taglib/include/taglib/taglib.h"
#include "ThirdParty/taglib/include/taglib/tstring.h"
#include "ThirdParty/taglib/include/taglib/flacfile.h"
#include "ThirdParty/taglib/include/taglib/wavfile.h"
#include "ThirdParty/taglib/include/taglib/mpegfile.h"
#include <vector>
#include "Audio/SongManager.hpp"
#include "Audio/Song.hpp"
#include "Renderables/VinylRecord.hpp"
#include "Game/Rendering/ShaderBootstrapper.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "UserData/AchievementManager.hpp"

TheGame* TheGame::instance = nullptr;
extern MeshBuilder* g_loadedMeshBuilder;
extern Skeleton* g_loadedSkeleton;
extern AnimationMotion* g_loadedMotion;
extern std::vector<AnimationMotion*>* g_loadedMotions;
extern int g_numLoadedMeshes;
extern int WINDOW_PHYSICAL_WIDTH;
extern int WINDOW_PHYSICAL_HEIGHT;
extern bool g_uiHidden;

unsigned int samplerID;
unsigned int diffuseID;
unsigned int normalMapID;
MeshRenderer* loadedMesh;
Material* lightMaterial;
const int NUM_LIGHTS = 1;
float spinFactor = 1.f;
static float animTime = 0.0f;

TheGame::TheGame()
    : m_jobConsumer(std::vector<JobType>(JobType::GENERIC_SLOW))
{
    SongManager::instance = new SongManager();
    AchievementManager::instance = new AchievementManager();
    UISystem::instance->LoadAndParseUIXML("Data/UI/PlayerLayout.xml");

    InitializeRand();
    SetUpShaders();
    m_blankFBOColorTexture = new Texture(WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT, Texture::TextureFormat::RGBA8);
    m_blankFBODepthTexture = new Texture(WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT, Texture::TextureFormat::D24S8);
    m_fbo = Framebuffer::FramebufferCreate(1, &m_blankFBOColorTexture, m_blankFBODepthTexture);

    m_fboMaterial = new Material(ShaderBootstrapper::compileShader("Data/Shaders/post.vert", "Data/Shaders/Backgrounds/earthbound.frag"),
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));
    m_fboMaterial->ReplaceSampler(Renderer::instance->CreateSampler(GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT));
    ShaderBootstrapper::initializeUniforms(m_fboMaterial);
    m_fboMaterial->SetDiffuseTexture(m_blankFBOColorTexture);
    m_fboMaterial->SetNormalTexture(Texture::CreateOrGetTexture("Data/Images/Logos/Logo.png"));

    MeshBuilder builder;
    builder.AddQuad(Vector3(-1, -1, 0), Vector3::UP, 2.0f, Vector3::RIGHT, 2.0f);
    m_quadForFBO = new MeshRenderer(new Mesh(), m_fboMaterial);
    builder.CopyToMesh(m_quadForFBO->m_mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);

    m_quadForFBO->m_material->SetFloatUniform("gPixelationFactor", 8.0f);
    Console::instance->m_backgroundTexture = Texture::CreateOrGetTexture("Data/Images/Logos/turntableSplash.png");

    InitializeUserDirectories();
    PrintConsoleWelcome();    
    LoadDefaultScene(); 
    InitializeMainCamera();
}

//-----------------------------------------------------------------------------------
TheGame::~TheGame()
{
    delete SongManager::instance;
    SongManager::instance = nullptr;
    delete AchievementManager::instance;
    AchievementManager::instance = nullptr;
    delete m_currentRecord;
    delete m_fbo->m_colorTargets[0];
    delete m_fbo->m_depthStencilTarget;
    delete m_quadForFBO->m_mesh;
    delete m_quadForFBO;
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
    for (unsigned int i = 0; i < NUM_PROC_GEN_MATERIALS; ++i)
    {
        delete m_proceduralGenerationMaterials[i]->m_shaderProgram;
        delete m_proceduralGenerationMaterials[i];
    }
    Framebuffer::FramebufferDelete(m_fbo);
}

//-----------------------------------------------------------------------------------
void TheGame::PrintConsoleWelcome()
{
    //Unicode support motivation
    Console::instance->PrintLine("Welcome to Turntable!", RGBA::GOLD);
    Console::instance->PrintLine("Version 0.3 - Rilassato", RGBA::GOLD);
    Console::instance->PrintLine("Type 'help' for a list of commands", RGBA::GOLD);
    
//     Console::instance->PrintLine("████████╗██╗   ██╗██████╗ ███╗   ██╗████████╗ █████╗ ██████╗ ██╗     ███████╗", RGBA::YELLOW);
//     Console::instance->PrintLine("╚══██╔══╝██║   ██║██╔══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔══██╗██║     ██╔════╝", RGBA::YELLOW);
//     Console::instance->PrintLine("   ██║   ██║   ██║██████╔╝██╔██╗ ██║   ██║   ███████║██████╔╝██║     █████╗  ", RGBA::YELLOW);
//     Console::instance->PrintLine("   ██║   ██║   ██║██╔══██╗██║╚██╗██║   ██║   ██╔══██║██╔══██╗██║     ██╔══╝  ", RGBA::YELLOW);
//     Console::instance->PrintLine("   ██║   ╚██████╔╝██║  ██║██║ ╚████║   ██║   ██║  ██║██████╔╝███████╗███████╗", RGBA::YELLOW);
//     Console::instance->PrintLine("   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝╚═════╝ ╚══════╝╚══════╝", RGBA::YELLOW);
}

//-----------------------------------------------------------------------------------
void TheGame::InitializeUserDirectories()
{
    //Ensure that our directory strucutre exists for user data.
    std::string appdata = GetAppDataDirectory();
    EnsureDirectoryExists(appdata + "\\Turntable");
    EnsureDirectoryExists(appdata + "\\Turntable\\Playlists");
    EnsureDirectoryExists(appdata + "\\Turntable\\Shaders");
}

//-----------------------------------------------------------------------------------
void TheGame::InitializeMainCamera()
{
    Camera3D* camera = ForwardRenderer::instance->GetMainCamera();
    camera->m_updateFromInput = false;
    camera->m_position = Vector3(30.0f, 30.0f, -10.0f);
    camera->LookAt(m_currentRecord->GetPosition());
}

//-----------------------------------------------------------------------------------
void TheGame::Update(float deltaSeconds)
{
    m_currentRecord->Update(deltaSeconds);
    SongManager::instance->Update(deltaSeconds);

    ShaderBootstrapper::updateUniforms(m_quadForFBO->m_material, deltaSeconds);

    if (!Console::instance->IsActive() && InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::TILDE))
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

    m_jobConsumer.Consume();
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

    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(axisLineLength, 0.0f, 0.0f), RGBA::RED);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, axisLineLength, 0.0f), RGBA::GREEN);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, axisLineLength), RGBA::BLUE);

    Renderer::instance->EnableDepthTest(false);

    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(axisLineLength, 0.0f, 0.0f), RGBA::RED);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, axisLineLength, 0.0f), RGBA::GREEN);
    Renderer::instance->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, axisLineLength), RGBA::BLUE);

    Renderer::instance->EnableDepthTest(true);
}

//-----------------------------------------------------------------------------------
void TheGame::SetUpShaders()
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
    m_testMaterial->SetDiffuseTexture("Data/Images/Logos/Logo.png");
    m_currentMaterial = m_testMaterial;

    m_proceduralGenerationMaterials[0] = new Material(
        new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/AlbumArtGen/albumArtHex.frag"),
            RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
            );

    m_proceduralGenerationMaterials[1] = new Material(
        new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/AlbumArtGen/albumArtSquare.frag"),
        RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
        );

    m_proceduralGenerationMaterials[2] = new Material(
        new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/AlbumArtGen/albumArtCheckerboard.frag"),
        RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
        );

    m_proceduralGenerationMaterials[3] = new Material(
        new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/AlbumArtGen/albumArtZigZag.frag"),
        RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
        );

    m_proceduralGenerationMaterials[4] = new Material(
        new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/AlbumArtGen/albumArtScales.frag"),
        RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
        );

    m_proceduralGenerationMaterials[5] = new Material(
        new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/AlbumArtGen/albumArtTriangles.frag"),
        RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
        );
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
    m_quadForFBO->m_material->SetMatrices(model, view, proj);
    m_quadForFBO->m_material->BindAvailableTextures();
    m_quadForFBO->Render();
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

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(printbackgrounds)
{
    Console::instance->PrintLine("Turntable Shaders:", RGBA::JOLTIK_YELLOW);
    std::vector<std::string> backgroundShaders = EnumerateFiles("Data/Shaders/Backgrounds/", "*.frag");
    for (std::string& shader : backgroundShaders)
    {
        Console::instance->PrintLine(shader, RGBA::JOLTIK_PURPLE);
    }

    Console::instance->PrintLine("User-Defined Shaders:", RGBA::JOLTIK_YELLOW);
    std::string appdata = GetAppDataDirectory();
    std::string userFilePath = Stringf("%s\\Turntable\\Shaders\\", appdata.c_str());
    std::vector<std::string> userBackgroundShaders = EnumerateFiles(userFilePath, "*.frag");
    if (userBackgroundShaders.empty())
    {
        Console::instance->PrintLine(Stringf("<None> (You can add some in the folder at %s)", userFilePath.c_str()), RGBA::KHAKI);
    }
    else
    {
        for (std::string& shader : userBackgroundShaders)
        {
            Console::instance->PrintLine(shader, RGBA::JOLTIK_PURPLE);
        }
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(setbackground)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("setbackground <shadername>", RGBA::RED);
        return;
    }
    std::string shaderName = args.GetStringArgument(0);
    shaderName = shaderName.substr(shaderName.size() - 5, 5) == ".frag" ? shaderName.substr(0, shaderName.size() - 5) : shaderName;

    std::string appdata = GetAppDataDirectory();
    std::string userFilePath = Stringf("%s\\Turntable\\Shaders\\%s.frag", appdata.c_str(), shaderName.c_str());
    std::string turntableFilePath = Stringf("Data/Shaders/Backgrounds/%s.frag", shaderName.c_str());
    std::string filePath = FileExists(userFilePath) ? userFilePath : turntableFilePath;
    if (!FileExists(filePath))
    {
        Console::instance->PrintLine("Could not find background shader with that name.", RGBA::RED);
        return;
    }
    
    delete TheGame::instance->m_fboMaterial->m_shaderProgram;
    delete TheGame::instance->m_fboMaterial;

    TheGame::instance->m_fboMaterial = new Material(ShaderBootstrapper::compileShader("Data/Shaders/post.vert", filePath.c_str()),
        RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));
    TheGame::instance->m_fboMaterial->SetDiffuseTexture(TheGame::instance->m_blankFBOColorTexture);
    TheGame::instance->m_fboMaterial->ReplaceSampler(Renderer::instance->CreateSampler(GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT));
    ShaderBootstrapper::initializeUniforms(TheGame::instance->m_fboMaterial);

    if (SongManager::instance->m_activeSong && SongManager::instance->m_activeSong->m_albumArt)
    {
        TheGame::instance->m_fboMaterial->SetNormalTexture(SongManager::instance->m_activeSong->m_albumArt);
    }
    else
    {
        TheGame::instance->m_fboMaterial->SetDiffuseTexture(Texture::CreateOrGetTexture("Data/Images/Logos/Logo.png"));
    }

    TheGame::instance->m_quadForFBO->m_material = TheGame::instance->m_fboMaterial;

    Console::instance->PrintLine("Successfuly changed background!", RGBA::FOREST_GREEN);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(hideui)
{
    UNUSED(args);
    g_uiHidden = true;
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(savedata)
{
    UNUSED(args);
    AchievementManager::instance->m_currentProfile->SaveToDisk();
    Console::instance->PrintLine("User data saved!", RGBA::FOREST_GREEN);
}