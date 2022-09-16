/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>
#include <dxgi.h>
#include <d3d11.h>

#include "Mesh.h"
#include "camera.h"
#include "Vertex.h"

struct SDL_Window;
struct SDL_Surface;

namespace Elite
{
	//-----------------------------------------------------
	// PrimitiveTopology enum class									
	//-----------------------------------------------------
	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);	// Constructor
		~Renderer();					// Destructor

		// -------------------------------------------------
		// Copy/move constructors and assignment operators
		// -------------------------------------------------    
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		//-------------------------------------------------
		// Member functions						
		//-------------------------------------------------
		//--- Update Functions ---
		void Update(float deltaT);
		void UpdateDirectX(float deltaT);

		void Render();
		void RenderDirectX();
		void RenderRasterizer();

		//--- Toggle Functions ---
		void ToggleDepthVisualization()
		{
			m_VisualizeDepthBuffer = !m_VisualizeDepthBuffer;
			const std::string tfText{ m_VisualizeDepthBuffer ? "true" : "false" };
			std::cout << "VisualizeDepthBuffer is " << tfText << std::endl;
		}
		void ToggleNormalMapping()
		{
			m_NormalMapping = !m_NormalMapping;
			const std::string tfText{ m_NormalMapping ? "true" : "false" };
			std::cout << "NormalMapping is " << tfText << std::endl;
		}
		void ToggleRotation()
		{
			m_Rotation = !m_Rotation;
			const std::string tfText{ m_Rotation ? "true" : "false" };
			std::cout << "Rotation is " << tfText << std::endl;
		}
		void ToggleVisibleFireMesh()
		{
			m_VisibleFireFX = !m_VisibleFireFX;
			const std::string tfText{ m_VisibleFireFX ? "Visible" : "Hidden" };
			std::cout << "FireFX is " << tfText << std::endl;
		}
		void ToggleDirectXUsage()
		{
			m_UseDirectX = !m_UseDirectX;
			const std::string tfText{ m_UseDirectX ? "DirectX" : "Software Rasterizer" };
			std::cout << "mesh is rendered by " << tfText << std::endl;
		}
		void ToggleSamplerState() const { m_pMesh->ToggleSamplerState(); }
		void ToggleCullState() const { m_pMesh->ToggleCullState(); }
	private:
		//-------------------------------------------------
		// Private member functions								
		//-------------------------------------------------
		void WorldToCamera();
		bool NotInFrustum(const FPoint4& pos) const;
		void NDCToRaster(FPoint4& pos);
		void PixelShading(const OutGoingVertex& v);

		OutGoingVertex CalcDepthVertex(const std::shared_ptr<Vertex>& vertex01, const std::shared_ptr<Vertex>& vertex02, const std::shared_ptr<Vertex>& vertex03, float w0, float w1, float w2);

		//--- functions ---
		void InitDirectX();
		void InitRasterizer();
		HRESULT InitializeDirectX();

		//-------------------------------------------------
		// Datamembers								
		//-------------------------------------------------
		//--- members ---
		SDL_Window* m_pWindow = nullptr;
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		float* m_pDepthBufferPixels = nullptr;
		PrimitiveTopology m_TopologyType = PrimitiveTopology::TriangleList;
		Camera* m_pCamera = nullptr;

		//--- Toggle members ---
		bool m_UseDirectX = true;
		bool m_NormalMapping = true;
		bool m_Rotation = true;
		bool m_VisualizeDepthBuffer = false;
		bool m_VisibleFireFX = true;
		bool m_IsInitialized;

		//--- directx 11 ---
		ID3D11Device* m_pDevice = nullptr;
		ID3D11DeviceContext* m_pDeviceContext = nullptr;
		IDXGIFactory* m_pDXGIFactory = nullptr;
		IDXGISwapChain* m_pSwapChain = nullptr;
		ID3D11Texture2D* m_pDepthStencilBuffer = nullptr;
		ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
		ID3D11Texture2D* m_pRenderTargetBuffer = nullptr;
		ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

		//--- Meshes ---
		Mesh* m_pMesh;
		Mesh* m_pFireMesh;

		//--- Rotation members ---
		FMatrix4 m_RotationMatrix = {};
		const float m_RotationSpeed{ float(E_PI/4.f) }; // 45degrees/s -> (pi/4)/s -> dt * pi / 4
		float m_RotationAngle{};

		//--- vector members ---
		std::vector<std::shared_ptr<Vertex>> m_Vertices;
		std::vector<uint32_t> m_Indices = {};

		//--- Textures members ---
		Texture_Rasterizer* m_pDiffuseTexture{ nullptr };
		Texture_Rasterizer* m_pNormalTexture{ nullptr };
		Texture_Rasterizer* m_pGlossMap{ nullptr };
		Texture_Rasterizer* m_pSpecularMap{ nullptr };
	};
}

#endif