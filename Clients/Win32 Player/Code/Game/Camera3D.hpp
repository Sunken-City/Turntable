#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Camera3D
{
public:

	Camera3D();
	void FixAndClampAngles(); //Prevents pitch from going above 89.9
	Vector3 GetForward() const;
	Vector3 GetForwardTwoComponent() const;
	Vector3 GetLeft() const;
	void UpdateViewFromCamera() const;

public:
	Vector3 m_position;
	EulerAngles m_orientation;
};
