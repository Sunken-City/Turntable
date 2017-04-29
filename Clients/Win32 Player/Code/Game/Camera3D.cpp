#include "Game/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

Camera3D::Camera3D()
: m_orientation(0.0f, 0.0f, 0.0f)
, m_position(0.0f, 0.0f, 0.0f)
{
}

Vector3 Camera3D::GetForward() const
{
	float cosYaw = cos(m_orientation.yawDegreesAboutZ);
	float sinYaw = sin(m_orientation.yawDegreesAboutZ);
	float cosPitch = cos(m_orientation.pitchDegreesAboutY);
	float sinPitch = sin(m_orientation.pitchDegreesAboutY);
	return Vector3(cosYaw * cosPitch, -sinPitch, sinYaw * cosPitch);
}

Vector3 Camera3D::GetForwardTwoComponent() const
{
	float cosYaw = cos(m_orientation.yawDegreesAboutZ);
	float sinYaw = sin(m_orientation.yawDegreesAboutZ);
	return (Vector3::FORWARD * cosYaw) + (Vector3::RIGHT * -sinYaw);
}

Vector3 Camera3D::GetLeft() const
{
	float cosYaw = cos(m_orientation.yawDegreesAboutZ);
	float sinYaw = sin(m_orientation.yawDegreesAboutZ);
	return (Vector3::RIGHT * -cosYaw) + (Vector3::FORWARD * -sinYaw);
}

void Camera3D::UpdateViewFromCamera() const
{
	Renderer::instance->Rotate(-m_orientation.pitchDegreesAboutY, 0.f, 1.f, 0.f);
	Renderer::instance->Rotate(-m_orientation.yawDegreesAboutZ, 0.f, 0.f, 1.f);
	Renderer::instance->Rotate(-m_orientation.rollDegreesAboutX, 1.f, 0.f, 0.f);
	Renderer::instance->Translate(-m_position.x, -m_position.y, -m_position.z);
}
