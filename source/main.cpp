#include "pch.h"
//#undef main
#ifdef _DEBUG
	#include <vld.h>
#endif

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;
	SDL_Window* pWindow = SDL_CreateWindow(
		"DualRasterizer - De Cooman Jarno",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow) };

	//--- INFO ---
	std::cout
		<< "----------------------------------------------" << std::endl
		<< "--- Movements ---" << std::endl
		<< "Move camera: WASD" << std::endl
		<< "Shift: double the movementspeed" << std::endl
		<< "Rotate: Right MouseButton" << std::endl
		<< "Fly: Left MouseButton" << std::endl
		<< "Up/Down: Right & Left MouseButton Drag" << std::endl << std::endl
		<< "--- Toggles ---" << std::endl
		<< "Toggle between DirectX and Software Rasterizer: E (Current: DirectX)" << std::endl
		<< "Toggle Rotation: R (Current: true)" << std::endl
		<< "Toggle CullModes: C (Current: Back-Face)" << std::endl
		<< "Toggle SamplerFilter (Only DirectX): F (Current Point)" << std::endl
		<< "Toggle FireFX (Only DirectX): T (Current Visible)" << std::endl
		<< "Toggle Depth Buffer Visualization (Only Rasterizer): Z (Current: false)" << std::endl
		<< "Toggle NormalMapping (Only Rasterizer): N (Current: true)" << std::endl
		<< "----------------------------------------------" << std::endl;
	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_Z)
					pRenderer->ToggleDepthVisualization();
				if (e.key.keysym.scancode == SDL_SCANCODE_N)
					pRenderer->ToggleNormalMapping();
				if (e.key.keysym.scancode == SDL_SCANCODE_R)
					pRenderer->ToggleRotation();
				if (e.key.keysym.scancode == SDL_SCANCODE_T)
					pRenderer->ToggleVisibleFireMesh();
				if (e.key.keysym.scancode == SDL_SCANCODE_E)
					pRenderer->ToggleDirectXUsage();
				if (e.key.keysym.scancode == SDL_SCANCODE_F)
					pRenderer->ToggleSamplerState();
				if (e.key.keysym.scancode == SDL_SCANCODE_C)
					pRenderer->ToggleCullState();
				break;
			}
		}
		//--------- Render ---------
		pRenderer->Update(pTimer->GetElapsed());
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			std::cout << "FPS: " << pTimer->GetFPS() << std::endl;
			printTimer -= 1.f;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	ShutDown(pWindow);
	return 0;
}