//---------------------------
// Includes
//---------------------------
#include "pch.h"
#include "Camera.h"
#include <SDL_keyboard.h>
#include <SDL.h>

//------------------------
//--- Member functions ---
//------------------------
void Camera::Update(float deltaT)
{
	//--- Camera Controls ---
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	//--- Movement ---
	const float cameraKeyboardSpeed = pKeyboardState[SDL_SCANCODE_LSHIFT] ? m_CameraMouseSpeed * 2.f : m_CameraMouseSpeed;

	MoveForward((pKeyboardState[SDL_SCANCODE_DOWN] - pKeyboardState[SDL_SCANCODE_UP]) * cameraKeyboardSpeed * deltaT);
	MoveForward((pKeyboardState[SDL_SCANCODE_S] - pKeyboardState[SDL_SCANCODE_W]) * cameraKeyboardSpeed * deltaT);

	MoveRight((pKeyboardState[SDL_SCANCODE_RIGHT] - pKeyboardState[SDL_SCANCODE_LEFT]) * cameraKeyboardSpeed * deltaT);
	MoveRight((pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A]) * cameraKeyboardSpeed * deltaT);

	//--- Rotation ---
	int x, y{};

	uint32_t mouseState = SDL_GetRelativeMouseState(&x, &y);
	if (mouseState == SDL_BUTTON_LMASK)
	{
		MoveForward(y * m_CameraMouseSpeed/10.f);
		Yaw(x * m_RotationSensitivity);
	}
	else if (mouseState == SDL_BUTTON_RMASK)
	{
		Pitch(y * m_RotationSensitivity);
		Yaw(x * m_RotationSensitivity);
	}
	else if (mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
	{
		MoveWorldUp(y * m_CameraMouseSpeed / 10.f);
	}
}

void Camera::MoveForward(float offSet)
{
	m_Position += m_W * offSet;
}
void Camera::MoveRight(float offSet)
{
	m_Position += m_U * offSet;
}
void Camera::MoveWorldUp(float offSet)
{
	m_Position += m_WorldUp * offSet;
}
void Camera::Pitch(float angle)
{
	const FMatrix3 rotation = MakeRotationX(angle * static_cast<float>(E_TO_RADIANS));
	m_W = Inverse(Transpose(rotation)) * m_W;
}
void Camera::Yaw(float angle)
{
	const FMatrix3 rotation = MakeRotationY(angle * static_cast<float>(E_TO_RADIANS));
	m_W = Inverse(Transpose(rotation)) * m_W;
}
FMatrix4 Camera::GetONB()
{
	Normalize(m_W);
	m_U = Cross(m_WorldUp, m_W);
	Normalize(m_U);
	m_V = Cross(m_W, m_U);
	Normalize(m_V);

	return FMatrix4(FVector4(m_U), FVector4(m_V), FVector4(-m_W), FVector4(FVector3(m_Position), 1.f));
}

Elite::FMatrix4 Camera::GetProjectionMatrix(float aspectRatio) const
{
	return FMatrix4(
		1.f / (aspectRatio * GetFOV()), 0, 0, 0,
		0, 1.f / GetFOV(), 0, 0,
		0, 0, m_FarPlane / (m_FarPlane - m_NearPlane), (m_FarPlane * m_NearPlane) / (m_NearPlane - m_FarPlane),
		0, 0, 1, 0);

}




