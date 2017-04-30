#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Matrix4x4.hpp"

class Camera3D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Camera3D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaTime);
    Vector3 GetForward() const;
    Vector3 GetForwardTwoComponent() const;
    Vector3 GetLeft() const;
    Matrix4x4 GetViewMatrix();

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static constexpr float BASE_MOVE_SPEED = 4.5f;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Vector3 m_position;
    EulerAngles m_orientation;
};
