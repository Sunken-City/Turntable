#include "Game/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/InputDevices/MouseInputDevice.hpp"

//-----------------------------------------------------------------------------------
Camera3D::Camera3D()
: m_orientation(0.0f, 0.0f, 0.0f)
, m_position(0.0f, 0.0f, 0.0f)
{
}

//-----------------------------------------------------------------------------------
Vector3 Camera3D::GetForward() const
{
    float cosYaw = cos(m_orientation.yawDegreesAboutZ);
    float sinYaw = sin(m_orientation.yawDegreesAboutZ);
    float cosPitch = cos(m_orientation.pitchDegreesAboutY);
    float sinPitch = sin(m_orientation.pitchDegreesAboutY);
    return Vector3(cosYaw * cosPitch, -sinPitch, sinYaw * cosPitch);
}

//-----------------------------------------------------------------------------------
Vector3 Camera3D::GetForwardTwoComponent() const
{
    float cosYaw = cos(m_orientation.yawDegreesAboutZ);
    float sinYaw = sin(m_orientation.yawDegreesAboutZ);
    return (Vector3::FORWARD * cosYaw) + (Vector3::RIGHT * -sinYaw);
}

//-----------------------------------------------------------------------------------
Vector3 Camera3D::GetLeft() const
{
    float cosYaw = cos(m_orientation.yawDegreesAboutZ);
    float sinYaw = sin(m_orientation.yawDegreesAboutZ);
    return (Vector3::RIGHT * -cosYaw) + (Vector3::FORWARD * -sinYaw);
}

//-----------------------------------------------------------------------------------
Matrix4x4 Camera3D::GetViewMatrix()
{
    //Set up view from camera
    Matrix4x4 view;
    Matrix4x4::MatrixMakeIdentity(&view);
    Matrix4x4::MatrixMakeRotationEuler(&view, -m_orientation.yawDegreesAboutZ, m_orientation.pitchDegreesAboutY, m_orientation.rollDegreesAboutX, m_position);
    Matrix4x4::MatrixInvertOrthogonal(&view);
    return view;
}

//-----------------------------------------------------------------------------------
void Camera3D::Update(float deltaTime)
{
    float moveSpeed;

    if (InputSystem::instance->IsKeyDown(InputSystem::ExtraKeys::SHIFT))
    {
        moveSpeed = BASE_MOVE_SPEED * 8.0f;
    }
    else
    {
        moveSpeed = BASE_MOVE_SPEED;
    }
    if (InputSystem::instance->IsKeyDown('W'))
    {
        Vector3 cameraForwardXY = GetForwardTwoComponent();
        m_position += cameraForwardXY * (moveSpeed * deltaTime);
    }
    if (InputSystem::instance->IsKeyDown('S'))
    {
        Vector3 cameraForwardXY = GetForwardTwoComponent();
        m_position -= cameraForwardXY * (moveSpeed * deltaTime);
    }
    if (InputSystem::instance->IsKeyDown('D'))
    {
        Vector3 cameraLeftXY = GetLeft();
        m_position -= cameraLeftXY * (moveSpeed * deltaTime);
    }
    if (InputSystem::instance->IsKeyDown('A'))
    {
        Vector3 camreaLeftXY = GetLeft();
        m_position += camreaLeftXY * (moveSpeed * deltaTime);
    }
    if (InputSystem::instance->IsKeyDown(' '))
    {
        m_position += Vector3::UP * (moveSpeed * deltaTime);
    }
    if (InputSystem::instance->IsKeyDown('Z'))
    {
        m_position -= Vector3::UP * (moveSpeed * deltaTime);
    }

    MouseInputDevice::CaptureMouseCursor();
    Vector2Int cursorDelta = InputSystem::instance->GetDeltaMouse();

    //Prevents pitch from going above 89.9
    m_orientation.yawDegreesAboutZ -= ((float)cursorDelta.x * 0.005f);
    float proposedPitch = m_orientation.pitchDegreesAboutY - ((float)cursorDelta.y * 0.005f);
    m_orientation.pitchDegreesAboutY = MathUtils::Clamp(proposedPitch, -3.14159f / 2.0f, 3.14159f / 2.0f);
}