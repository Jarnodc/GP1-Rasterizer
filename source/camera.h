#pragma once

using namespace Elite;
class Camera final
{
public:
	Camera() = default;					// Constructor
	~Camera() = default;				// Destructor

	// -------------------------------------------------
	// CopCamera/move constructors and assignment operators
	// -------------------------------------------------    
	Camera(const Camera& other) = delete;
	Camera(Camera&& other) noexcept = delete;
	Camera& operator=(const Camera& other) = delete;
	Camera& operator=(Camera&& other) noexcept = delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void Update(float deltaT);

	Elite::FMatrix4 GetONB();
	Elite::FMatrix4 GetProjectionMatrix(float aspectRatio) const;
private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	void MoveForward(float offSet);
	void MoveRight(float offSet);
	void MoveWorldUp(float offSet);
	void Pitch(float angle);
	void Yaw(float angle);

	float GetFOV() const { return tan(m_FOVAngle * static_cast<float>(E_TO_RADIANS) / 2.f); }

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	FPoint3 m_Position{ 0.f, 0.f, 0.f };
	FVector3 m_WorldUp{ 0.f, 1.f, 0.f };
	FVector3 m_U{ 1.f, 0.f, 0.f };		//ONB x-axis
	FVector3 m_V{ 0.f, 1.f, 0.f };		//ONB y-axis
	FVector3 m_W{ 0.f, 0.f, 1.f };		//ONB z-axis
	float m_FOVAngle{ 45 };

	float m_CameraMouseSpeed = 10.f;
	float m_RotationSensitivity = 0.075f;

	float m_NearPlane = 0.1f;
	float m_FarPlane = 100.f;

};

