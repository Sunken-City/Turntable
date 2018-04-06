#include "Game/Renderables/VinylRecord.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/3D/Renderable3D.hpp"
#include "Engine/Renderer/3D/Scene3D.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/3D/ForwardRenderer.hpp"
#include "Engine/Renderer/3D/Camera3D.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"

const Vector3 VinylRecord::VINYL_SPAWN_POSITION = Vector3(30.0f, 0.0f, 30.0f);

//-----------------------------------------------------------------------------------
VinylRecord::VinylRecord()
{
    InitializeMeshes();
}

//-----------------------------------------------------------------------------------
VinylRecord::VinylRecord(Type type)
    : m_type(type)
    , m_baseRPM(GetRPMFromType(type))
{
    InitializeMeshes();
}

//-----------------------------------------------------------------------------------
VinylRecord::~VinylRecord()
{
    delete m_innerMaterial->m_shaderProgram;
    delete m_outerMaterial->m_shaderProgram;
    delete m_sleeveMaterial->m_shaderProgram;
    delete m_innerMaterial;
    delete m_outerMaterial;
    delete m_sleeveMaterial;
    delete m_vinylLabel->m_meshRenderer.m_mesh;
    delete m_vinyl->m_meshRenderer.m_mesh;
    delete m_sleeve->m_meshRenderer.m_mesh;
    delete m_vinyl;
    delete m_vinylLabel;
    delete m_sleeve;
}

//-----------------------------------------------------------------------------------
float VinylRecord::GetRPMFromType(Type type)
{
    switch (type)
    {
    case Type::RPM_45:
        return 45.0f;
    case Type::RPM_33:
        return 33.333333f;
    case Type::RPM_45_FLAT:
        return 45.0f;
    case Type::RPM_33_FLAT:
        return 33.333333f;
    default:
        ERROR_AND_DIE("No RPM information for type. (Did you just add a new type of record?)");
    }
}

//-----------------------------------------------------------------------------------
void VinylRecord::Update(float deltaSeconds)
{
    UpdateVinylRotation(deltaSeconds);
    UpdateVinylJacket();

    m_innerMaterial->SetVec4Uniform(std::hash<std::string>{}("gColor"), RGBA::WHITE.ToVec4());
    m_innerMaterial->SetVec4Uniform(std::hash<std::string>{}("gAmbientLight"), RGBA::BLACK.ToVec4());
    m_innerMaterial->SetVec4Uniform(std::hash<std::string>{}("gLightColor"), RGBA::WHITE.ToVec4());
    m_innerMaterial->SetVec4Uniform(std::hash<std::string>{}("gFogColor"), RGBA::GRAY.ToVec4());
    m_innerMaterial->SetVec3Uniform(std::hash<std::string>{}("gLightPosition"), Vector3(30.0f, 10.0f, 30.0f));
    m_innerMaterial->SetVec3Uniform(std::hash<std::string>{}("gCameraPosition"), ForwardRenderer::instance->GetMainCamera()->m_position);
    m_innerMaterial->SetFloatUniform(std::hash<std::string>{}("gLightIntensity"), 150.0f);
    m_innerMaterial->SetFloatUniform(std::hash<std::string>{}("gSpecularPower"), 8.0f);
    m_innerMaterial->SetFloatUniform(std::hash<std::string>{}("gMinFogDistance"), 50.0f);
    m_innerMaterial->SetFloatUniform(std::hash<std::string>{}("gMaxFogDistance"), 50.2f);
    m_innerMaterial->SetFloatUniform(std::hash<std::string>{}("gTime"), (float)GetCurrentTimeSeconds());

    m_outerMaterial->SetVec4Uniform(std::hash<std::string>{}("gColor"), RGBA::WHITE.ToVec4());
    m_outerMaterial->SetVec4Uniform(std::hash<std::string>{}("gAmbientLight"), RGBA::BLACK.ToVec4());
    m_outerMaterial->SetVec4Uniform(std::hash<std::string>{}("gLightColor"), RGBA::WHITE.ToVec4());
    m_outerMaterial->SetVec4Uniform(std::hash<std::string>{}("gFogColor"), RGBA::GRAY.ToVec4());
    m_outerMaterial->SetVec3Uniform(std::hash<std::string>{}("gLightPosition"), Vector3(30.0f, 10.0f, 30.0f));
    m_outerMaterial->SetVec3Uniform(std::hash<std::string>{}("gCameraPosition"), ForwardRenderer::instance->GetMainCamera()->m_position);
    m_outerMaterial->SetFloatUniform(std::hash<std::string>{}("gLightIntensity"), 150.0f);
    m_outerMaterial->SetFloatUniform(std::hash<std::string>{}("gSpecularPower"), 8.0f);
    m_outerMaterial->SetFloatUniform(std::hash<std::string>{}("gMinFogDistance"), 50.0f);
    m_outerMaterial->SetFloatUniform(std::hash<std::string>{}("gMaxFogDistance"), 50.2f);
    m_outerMaterial->SetFloatUniform(std::hash<std::string>{}("gTime"), (float)GetCurrentTimeSeconds());

}

//-----------------------------------------------------------------------------------
void VinylRecord::AddToScene(Scene3D* scene)
{
    scene->RegisterRenderable(m_vinylLabel);
    scene->RegisterRenderable(m_vinyl);
    scene->RegisterRenderable(m_sleeve);
}

//-----------------------------------------------------------------------------------
void VinylRecord::RemoveFromScene(Scene3D* scene)
{
    scene->UnregisterRenderable(m_vinylLabel);
    scene->UnregisterRenderable(m_vinyl);
    scene->UnregisterRenderable(m_sleeve);
}

//-----------------------------------------------------------------------------------
Vector3 VinylRecord::GetPosition() const
{
   return m_vinyl->m_transform.GetWorldPosition();
}

//-----------------------------------------------------------------------------------
void VinylRecord::InitializeMeshes()
{
    if (m_type == RPM_45 || m_type == RPM_45_FLAT)
    {
        Mesh* innerMesh = nullptr;
        Mesh* outerMesh = nullptr;
        if (m_type == RPM_45_FLAT)
        {
            innerMesh = MeshBuilder::LoadMesh("data/fbx/vinyl/45rpmflat_1.picomesh");
            outerMesh = MeshBuilder::LoadMesh("data/fbx/vinyl/45rpmflat_0.picomesh");
        }
        else
        {
            innerMesh = MeshBuilder::LoadMesh("data/fbx/vinyl/45rpm_1.picomesh");
            outerMesh = MeshBuilder::LoadMesh("data/fbx/vinyl/45rpm_0.picomesh");
        }
        Mesh* sleeveMesh = MeshBuilder::LoadMesh("data/fbx/vinyl/45sleeve_0.picomesh");

        m_innerMaterial = new Material(
            new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/basicLight.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
            RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
            );
        m_outerMaterial = new Material(
            new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/basicLight.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
            RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
            );
        m_sleeveMaterial = new Material(
            new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/fixedVertexFormat.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
            RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
            );

        m_innerMaterial->SetDiffuseTexture("Data/Images/LabelTextures/45RPMLabel.tga");
        m_outerMaterial->SetDiffuseTexture("Data/Images/DiscTextures/45RPMBaseColor.png");
        m_outerMaterial->SetNormalTexture("Data/Images/DiscTextures/45RPMSpec.png");
        m_sleeveMaterial->SetDiffuseTexture("Data/Images/SleeveTextures/Generic45Sleeve.tga");

        m_vinylLabel = new Renderable3D(innerMesh, m_innerMaterial);
        m_sleeve = new Renderable3D(sleeveMesh, m_sleeveMaterial);
        m_vinyl = new Renderable3D(outerMesh, m_outerMaterial);

        m_vinyl->m_transform.AddChild(&m_vinylLabel->m_transform);
        m_vinyl->m_transform.AddChild(&m_sleeve->m_transform);
        m_vinyl->m_transform.SetPosition(VINYL_SPAWN_POSITION);

        m_sleeve->m_transform.IgnoreParentRotation();
        m_sleeve->m_transform.SetRotationDegrees(Vector3(90.0f, 180.0f, 0.0f));
        m_sleeve->m_transform.SetScale(Vector3(1.0f, 1.0f, 0.75f));
        m_sleeve->m_transform.SetPosition(Vector3(0.0f, 0.4f, 0.0f));
    }
    else if(m_type == RPM_33 || m_type == RPM_33_FLAT)
    {
        Mesh* innerMesh = nullptr;
        Mesh* outerMesh = nullptr;
        if (m_type == RPM_33_FLAT)
        {
            innerMesh = MeshBuilder::LoadMesh("data/fbx/vinyl/33rpmflat_1.picomesh");
            outerMesh = MeshBuilder::LoadMesh("data/fbx/vinyl/33rpmflat_0.picomesh");
        }
        else
        {
            innerMesh = MeshBuilder::LoadMesh("data/fbx/vinyl/33rpm_1.picomesh");
            outerMesh = MeshBuilder::LoadMesh("data/fbx/vinyl/33rpm_0.picomesh");
        }
        Mesh* sleeveMesh = MeshBuilder::LoadMesh("data/fbx/vinyl/33sleeve_0.picomesh");

        m_innerMaterial = new Material(
            new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/basicLight.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
            RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
            );
        m_outerMaterial = new Material(
            new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/basicLight.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
            RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
            );
        m_sleeveMaterial = new Material(
            new ShaderProgram("Data/Shaders/basicLight.vert", "Data/Shaders/fixedVertexFormat.frag"), //SkinDebug fixedVertexFormat timeBased basicLight multiLight
            RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
            );

        m_innerMaterial->SetDiffuseTexture("Data/Images/LabelTextures/33RPMLabel.tga");
        m_outerMaterial->SetDiffuseTexture("Data/Images/DiscTextures/33RPMBaseColor.png");
        m_outerMaterial->SetNormalTexture("Data/Images/DiscTextures/33RPMSpec.png");
        m_sleeveMaterial->SetDiffuseTexture("Data/Images/SleeveTextures/Generic33Sleeve.tga");

        m_vinylLabel = new Renderable3D(innerMesh, m_innerMaterial);
        m_sleeve = new Renderable3D(sleeveMesh, m_sleeveMaterial);
        m_vinyl = new Renderable3D(outerMesh, m_outerMaterial);

        m_vinyl->m_transform.AddChild(&m_vinylLabel->m_transform);
        m_vinyl->m_transform.AddChild(&m_sleeve->m_transform);
        m_vinyl->m_transform.SetPosition(VINYL_SPAWN_POSITION);

        m_sleeve->m_transform.IgnoreParentRotation();
        m_sleeve->m_transform.SetRotationDegrees(Vector3(90.0f, 180.0f, 0.0f));
        m_sleeve->m_transform.SetScale(Vector3(1.0f, 1.0f, 0.75f));
        m_sleeve->m_transform.SetPosition(Vector3(0.0f, 0.4f, 0.0f));
    }
    else
    {
        ERROR_AND_DIE("Invalid type set for VinylRecord");
    }
}

//-----------------------------------------------------------------------------------
void VinylRecord::UpdateVinylRotation(float deltaSeconds)
{
    static float currentRotationRate = m_currentRotationRate;

    currentRotationRate = MathUtils::Lerp(0.1f, currentRotationRate, m_currentRotationRate);
    float rotationThisFrame = currentRotationRate * deltaSeconds;
    Vector3 newRotation = m_vinyl->m_transform.GetWorldRotationDegrees();
    newRotation.y += rotationThisFrame;
    m_vinyl->m_transform.SetRotationDegrees(newRotation);
}

//-----------------------------------------------------------------------------------
void VinylRecord::UpdateVinylJacket()
{
    static bool jacketOn = false;
    static Vector3 desiredJacketPosition = m_sleeve->m_transform.GetLocalPosition() + Vector3(-100.0f, 0.0f, 0.0f);
    if (InputSystem::instance->WasKeyJustPressed('J'))
    {
        jacketOn = !jacketOn;

        desiredJacketPosition.x = jacketOn ? 0.0f : -100.0f;
    }

    Vector3 currentPosition = MathUtils::Lerp(0.1f, m_sleeve->m_transform.GetLocalPosition(), desiredJacketPosition);
    m_sleeve->m_transform.SetPosition(currentPosition);
}
