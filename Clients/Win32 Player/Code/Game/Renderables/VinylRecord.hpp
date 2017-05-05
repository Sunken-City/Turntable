#pragma once

class Renderable3D;
class Material;
class Scene3D;

class VinylRecord
{
public:
    enum Type
    {
        RPM_45,
        RPM_33
    };

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    VinylRecord();
    VinylRecord(Type type);
    ~VinylRecord();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void AddToScene(Scene3D* scene);

private:
    void InitializeMeshes();
    void UpdateVinylRotation(float deltaSeconds);
    void UpdateVinylJacket();

public:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Renderable3D* m_vinyl = nullptr;
    Renderable3D* m_vinylLabel = nullptr;
    Renderable3D* m_sleeve = nullptr;
    Material* m_innerMaterial;
    Material* m_outerMaterial;
    Material* m_sleeveMaterial;
    float m_currentRotationRate = 0.0f;
    Type m_type;
};