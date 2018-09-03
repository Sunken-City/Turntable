#pragma once

class Renderable3D;
class Material;
class Scene3D;
class Vector3;
class Texture;

class VinylRecord
{
public:
    enum Type
    {
        RPM_45,
        RPM_33,
        RPM_45_FLAT,
        RPM_33_FLAT,
    };

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    VinylRecord();
    VinylRecord(Type type);
    ~VinylRecord();

    //STATIC FUNCTIONS/////////////////////////////////////////////////////////////////////
    float GetRPMFromType(Type type);

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void AddToScene(Scene3D* scene);
    void RemoveFromScene(Scene3D* scene);
    void SetAlbumTexture(Texture* texture);
    Vector3 GetPosition() const;

private:
    void InitializeMeshes();
    void UpdateVinylRotation(float deltaSeconds);
    void UpdateVinylJacket();

public:
    //CONSTANTS///////////////////////////////////////////////////////////////////////////////
    static const Vector3 VINYL_SPAWN_POSITION;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Renderable3D* m_vinyl = nullptr;
    Renderable3D* m_vinylLabel = nullptr;
    Renderable3D* m_sleeve = nullptr;
    Material* m_innerMaterial;
    Material* m_outerMaterial;
    Material* m_sleeveMaterial;
    float m_currentRotationRate = 0.0f;
    float m_baseRPM = 45.0f;
    Type m_type;
};