#include "pch.h"

//Project includes
#include "ERenderer.h"
#include "EOBJParser.h"
#include "Effect.h"
#include "FireEffect.h"
#include "Texture.h"

Renderer::Renderer(SDL_Window* pWindow)
	: m_pWindow{pWindow}
	, m_IsInitialized{false}
{
	int width, height = 0;
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);

	m_pCamera = new Camera();

	InitDirectX();
	InitRasterizer();
}

Renderer::~Renderer()
{
	delete m_pMesh;
	delete m_pFireMesh;
	delete m_pCamera;
	m_pMesh = nullptr;
	m_pFireMesh = nullptr;
	m_pCamera = nullptr;

	if (m_pDevice)
		m_pDevice->Release();
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}

	if (m_pDXGIFactory)
		m_pDXGIFactory->Release();

	if (m_pSwapChain)
		m_pSwapChain->Release();

	if (m_pDepthStencilBuffer)
		m_pDepthStencilBuffer->Release();

	if (m_pDepthStencilView)
		m_pDepthStencilView->Release();

	if (m_pRenderTargetBuffer)
		m_pRenderTargetBuffer->Release();

	if (m_pRenderTargetView)
		m_pRenderTargetView->Release();
	delete[] m_pDepthBufferPixels;
	m_pDepthBufferPixels = nullptr;

	//--- Delete Textures ---
	delete m_pDiffuseTexture;
	m_pDiffuseTexture = nullptr;

	delete m_pNormalTexture;
	m_pNormalTexture = nullptr;

	delete m_pGlossMap;
	m_pGlossMap = nullptr;

	delete m_pSpecularMap;
	m_pSpecularMap = nullptr;

	m_Vertices.clear();
	m_Indices.clear();
}

void Renderer::Update(float deltaT)
{
	//-- Update Camera --
	m_pCamera->Update(deltaT);

	//-- Rotate RotationMatrix --
	if (m_Rotation)
		m_RotationAngle += deltaT * m_RotationSpeed;

	m_RotationMatrix = MakeRotationY(m_RotationAngle);
	m_RotationMatrix[3].xyz = m_pMesh->GetPosition();

	if (m_UseDirectX)
		UpdateDirectX(deltaT);
}

void Renderer::UpdateDirectX(float deltaT)
{
	// Set Mesh Position
	const auto meshPos{m_pMesh->GetPosition()};
	m_RotationMatrix[3][0] = meshPos.x;
	m_RotationMatrix[3][1] = meshPos.y;
	m_RotationMatrix[3][2] = meshPos.z;
	m_RotationMatrix[3][3] = 1;

	//-- variables --
	const float aspectRatio = static_cast<float>(m_Width) / m_Height;
	FMatrix4 WorldViewProjectionMatrix = m_pCamera->GetProjectionMatrix(aspectRatio) * Inverse(m_pCamera->GetONB());

	//-- pass matrix --
	m_pMesh->SetWorldProjMat(&WorldViewProjectionMatrix[0][0]);
	m_pMesh->SetViewInvMat(&m_pCamera->GetONB()[0][0]);
	m_pMesh->SetWorldMat(&m_RotationMatrix[0][0]);
	if (m_VisibleFireFX)
	{
		m_pFireMesh->SetWorldProjMat(&WorldViewProjectionMatrix[0][0]);
		m_pFireMesh->SetViewInvMat(&m_pCamera->GetONB()[0][0]);
		m_pFireMesh->SetWorldMat(&m_RotationMatrix[0][0]);
	}
}

void Renderer::Render()
{
	if (m_UseDirectX)
		RenderDirectX();
	else
		RenderRasterizer();
}

void Renderer::RenderDirectX()
{
	if (!m_IsInitialized)
		return;

	//Clear Buffers
	auto clearColor = RGBColor(0.1f, 0.1f, 0.1f);
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	//Render
	//...
	m_pMesh->Render(m_pDeviceContext);

	if (m_VisibleFireFX)
		m_pFireMesh->Render(m_pDeviceContext);

	//Present
	m_pSwapChain->Present(0, 0);
}

void Renderer::RenderRasterizer()
{
	SDL_LockSurface(m_pBackBuffer);

	//--- set Every Pixel black ---
	for (uint32_t r = 0; r < m_Height; ++r)
	{
		for (uint32_t c = 0; c < m_Width; ++c)
		{
			//--- fill depthbuffer with maximum float ---
			m_pDepthBufferPixels[c + (r * m_Width)] = FLT_MAX;
			m_pBackBufferPixels[c + (r * m_Width)] = GetSDL_ARGBColor(RGBColor{0.1f, 0.1f, 0.1f});
		}
	}

	//--- transform all vertices ---
	WorldToCamera();

	//--- Loop over every number in indices ---
	const auto size{m_TopologyType == PrimitiveTopology::TriangleList ? m_Indices.size() : m_Indices.size() - 2};

	for (size_t i = 0; i < size;)
	{
		//--- Check for calculation index ---
		const int evenIndex{m_TopologyType == PrimitiveTopology::TriangleStrip ? static_cast<int>(i % 2) : 0};

		const uint32_t index0 = m_Indices[i];
		const uint32_t index1 = m_Indices[i + 1 + evenIndex];
		const uint32_t index2 = m_Indices[i + 2 - evenIndex];

		//--- Loop based on topology ---
		if (m_TopologyType == PrimitiveTopology::TriangleList)
			i += 3;
		else if (m_TopologyType == PrimitiveTopology::TriangleStrip)
			i += 1;

		//--- Use screenPostion For calculation ---
		FPoint4 v0 = m_Vertices.at(index0)->screenPosition;
		FPoint4 v1 = m_Vertices.at(index1)->screenPosition;
		FPoint4 v2 = m_Vertices.at(index2)->screenPosition;

		//--- Check if in frustrum or not. When not in frustrum -> continue ---
		if (NotInFrustum(v0) || NotInFrustum(v1) || NotInFrustum(v2))
			continue;

		//--- NDC to Raster Space ---
		NDCToRaster(v0);
		NDCToRaster(v1);
		NDCToRaster(v2);

		//--- Calc Bounding Box ---
		const float left = std::min(std::min(v0.x, v1.x), v2.x);
		const float Top = std::min(std::min(v0.y, v1.y), v2.y);
		const float Right = std::max(std::max(v0.x, v1.x), v2.x);
		const float Bottom = std::max(std::max(v0.y, v1.y), v2.y);
		auto bb = BoundingBox(
			FPoint2(std::max(0.f, left), std::max(0.f, Top)),
			FPoint2(std::min(m_Width - 1.f, Right), std::min(m_Height - 1.f, Bottom)));


		//--- Loop over all the pixels in the boundingBox ---
		//start Top and end when bottom of boundingBox and is smaller then uw total height of screen
		for (uint32_t r = static_cast<uint32_t>(bb.topLeft.y); static_cast<int>(r) < bb.rightBottom.y && r < m_Height;
		     ++r)
		{
			//start left and end when right of boundingBox and is smaller then uw total height of screen
			for (uint32_t c = static_cast<uint32_t>(bb.topLeft.x); static_cast<int>(c) < bb.rightBottom.x && c < m_Width
			     ; ++c)
			{
				const FPoint2 curpixel{static_cast<float>(c), static_cast<float>(r)};

				const auto edge0 = FVector2(v1 - v0);
				const FVector2 pointToEdge0 = curpixel - FPoint2(v0);

				const auto edge1 = FVector2(v2 - v1);
				const FVector2 pointToEdge1 = curpixel - FPoint2(v1);

				const auto edge2 = FVector2(v0 - v2);
				const FVector2 pointToEdge2 = curpixel - FPoint2(v2);

				//Calc culling
				float w2 = Cross(pointToEdge0, edge0);
				float w0 = Cross(pointToEdge1, edge1);
				float w1 = Cross(pointToEdge2, edge2);
				if (BaseEffect::CullMode::front == m_pMesh->GetCullMode())
				{
					w2 *= -1;
					w0 *= -1;
					w1 *= -1;
				}
				if (w0 >= 0 && w1 >= 0 && w2 >= 0 || (BaseEffect::CullMode::no == m_pMesh->GetCullMode() && (w0 >= 0 && w1 >= 0 && w2 >= 0 || w0 < 0 && w1 < 0 && w2 < 0)))
				{
					//--- Barycentric Coordinates ---
					const float totalArea = abs(Cross(-edge0, edge2));
					w0 /= totalArea;
					w1 /= totalArea;
					w2 /= totalArea;

					float zBuffer{(w0 / v0.z) + (w1 / v1.z) + (w2 / v2.z)};
					zBuffer = 1 / zBuffer;

					if (zBuffer > 1.f || zBuffer < 0.f)
						break;

					if (zBuffer < m_pDepthBufferPixels[c + (r * m_Width)])
					{
						m_pDepthBufferPixels[c + (r * m_Width)] = zBuffer;

						OutGoingVertex v{
							CalcDepthVertex(m_Vertices.at(index0), m_Vertices.at(index1), m_Vertices.at(index2), w0, w1,
							                w2)
						};
						v.vertex.Position = FPoint4(curpixel, zBuffer, v.depthInterpolated);

						PixelShading(v);
					}
				}
			}
		}
	}
	if (m_VisualizeDepthBuffer)
	{
		for (int i = 0; i < static_cast<int>(m_Width * m_Height); ++i)
		{
			float depthValue = m_pDepthBufferPixels[i];
			float depthRemapped = Remap(depthValue, 0.997f, 1.f);
			RGBColor finalColor{ depthRemapped, depthRemapped, depthRemapped };
			finalColor.MaxToOne();
			m_pBackBufferPixels[i] = GetSDL_ARGBColor(finalColor);
		}
	}

	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, nullptr, m_pFrontBuffer, nullptr);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::InitDirectX()
{
	//Initialize DirectX pipeline
	if (InitializeDirectX() == S_OK)
	{
		m_IsInitialized = true;
		std::cout << "DirectX is ready\n";
	}
	else
		std::cout << "Failed to initializeDirectX\n";

	Texture fire("Resources/fireFX_diffuse.png", m_pDevice);
	Texture diff("Resources/vehicle_diffuse.png", m_pDevice);
	Texture spec("Resources/vehicle_specular.png", m_pDevice);
	Texture norm("Resources/vehicle_normal.png", m_pDevice);
	Texture glos("Resources/vehicle_gloss.png", m_pDevice);

	std::vector<Mesh::Base_Vertex> vertices;

	//--- fill vertices and indices ---
	ParseOBJ("Resources/vehicle.obj", vertices, m_Indices);
	auto temp{new Effect(m_pDevice, L"Resources/PosCol3D.fx")};
	m_pMesh = new Mesh(m_pDevice, temp, vertices, m_Indices, {0, 0, -50});
	m_pMesh->SetMaps(diff.GetTextureResourceView(), norm.GetTextureResourceView(), spec.GetTextureResourceView(),
	                 glos.GetTextureResourceView());

	for (auto v : vertices)
	{
		auto pV{std::make_shared<Vertex>(v)};
		m_Vertices.push_back(pV);
	}

	std::vector<Mesh::Base_Vertex> fireVertices;
	std::vector<uint32_t> fireIndices;

	auto tempfire{new FireEffect(m_pDevice, L"Resources/PosCol3D_FireEffect.fx")};
	ParseOBJ("Resources/fireFX.obj", fireVertices, fireIndices);
	m_pFireMesh = new Mesh(m_pDevice, tempfire, fireVertices, fireIndices, {0, 0, -50});

	m_pFireMesh->SetMaps(fire.GetTextureResourceView(), norm.GetTextureResourceView(), spec.GetTextureResourceView(),
	                     glos.GetTextureResourceView());

	m_pDeviceContext->GenerateMips(fire.GetTextureResourceView());
	m_pDeviceContext->GenerateMips(diff.GetTextureResourceView());
	m_pDeviceContext->GenerateMips(spec.GetTextureResourceView());
	m_pDeviceContext->GenerateMips(norm.GetTextureResourceView());
	m_pDeviceContext->GenerateMips(glos.GetTextureResourceView());
}

void Renderer::InitRasterizer()
{
	//Initialize
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = static_cast<uint32_t*>(m_pBackBuffer->pixels);

	//--- Set Textures ---
	m_pDiffuseTexture = new Texture_Rasterizer("Resources/vehicle_diffuse.png");
	m_pNormalTexture = new Texture_Rasterizer("Resources/vehicle_normal.png");
	m_pGlossMap = new Texture_Rasterizer("Resources/vehicle_gloss.png");
	m_pSpecularMap = new Texture_Rasterizer("Resources/vehicle_specular.png");

	//--- Create array with size of all pixels ---
	m_pDepthBufferPixels = new float[m_Width * m_Height]{};
}

HRESULT Renderer::InitializeDirectX()
{
	//create device context
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) ||defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0,
	                                   D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);
	if (FAILED(result)) return result;

	//create swapchain
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));
	if (FAILED(result)) return result;

	//Create SwapChain Descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//Get the handle HWND from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	//Create SwapChain and hook it into the handle of the SDL window
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result)) return result;

	//Create the Depth/Stencil Buffer and View
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//create depth stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;

	// Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	//Bind the Views to the Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the Viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);

	return result;
}

void Renderer::WorldToCamera()
{
	//--- const variables ---
	const float aspectRatio = m_Width / static_cast<float>(m_Height);

	//--- Camera settings ---
	const FMatrix4 inversedONB = Inverse(m_pCamera->GetONB());
	const FMatrix4 projectionMatrix = m_pCamera->GetProjectionMatrix(aspectRatio);

	FMatrix4 WorldViewProjectionMatrix{projectionMatrix * inversedONB};
	const auto cameraPosition = FVector3(inversedONB(0, 3), inversedONB(1, 3), inversedONB(2, 3));

	//--- Transform World To screen ---
	for (auto& vertex : m_Vertices)
	{
		FPoint4 curScPos{WorldViewProjectionMatrix * m_RotationMatrix * vertex->vertex.Position};
		//--- viewSpace to projection ---
		curScPos.x /= curScPos.w;
		curScPos.y /= curScPos.w;
		curScPos.z /= curScPos.w;

		FVector3 viewDirection = FVector4(m_RotationMatrix * (vertex->vertex.Position + FVector4{0, 0, -50})).xyz -
			cameraPosition;
		Normalize(viewDirection);

		vertex->viewDirection = viewDirection;
		vertex->screenPosition = curScPos;
	}
}

bool Renderer::NotInFrustum(const FPoint4& pos) const
{
	return pos.x < -1.f || pos.x > 1.f || pos.y < -1.f || pos.y > 1.f || pos.z < 0.f || pos.z > 1.f;
}

void Renderer::NDCToRaster(FPoint4& pos)
{
	pos.x = (pos.x + 1) / 2.f * m_Width;
	pos.y = (1 - pos.y) / 2.f * m_Height;
}

void Renderer::PixelShading(const OutGoingVertex& v)
{
	//--- Light Constants ---
	const float lightIntensity = 7.0f;
	const FVector3 lightDirection = {.577f, -.577f, -.577f};

	//--- PixelShading Constants ---
	const FVector3 tangent{FMatrix3(m_RotationMatrix) * v.vertex.Tangent}, normal{ FMatrix3(m_RotationMatrix) * v.vertex.Normal};

	//--- NORMAL MAPPING ---
	const FVector3 binormal{GetNormalized(Cross(tangent, normal))};
	FMatrix3 tangentSpaceAxis = FMatrix3(tangent, binormal, normal);

	const RGBColor normalMap = {m_pNormalTexture->Sample(v.vertex.Uv)};
	const FVector3 sampledValue = {2.f * normalMap.r - 1.f, 2.f * normalMap.g - 1.f, 2.f * normalMap.b - 1.f};
	const FVector3 newNormal{!m_NormalMapping ? normal : tangentSpaceAxis * sampledValue};

	//--- DIFFUSE COLOR ---
	float observedArea = Dot(-newNormal, lightDirection);
	observedArea = std::max(0.f, observedArea);
	observedArea *= lightIntensity / static_cast<float>(E_PI);
	RGBColor diffuseColor = {m_pDiffuseTexture->Sample(v.vertex.Uv) * observedArea};
	diffuseColor.MaxToOne();

	//--- PHONG SPECULAR ---
	const float shininess = 25.f;
	const FVector3 reflect = Reflect(-lightDirection, newNormal);
	float specularStrength = Dot(v.viewDirection, reflect);
	specularStrength = Clamp(specularStrength, 0.f, 1.f);
	const RGBColor glossMap = {m_pGlossMap->Sample(v.vertex.Uv)};
	specularStrength = std::pow(specularStrength, glossMap.r * shininess);
	RGBColor specularColor = {m_pSpecularMap->Sample(v.vertex.Uv) * specularStrength};
	specularColor.MaxToOne();

	//--- AMBIENT COLOR ---
	const RGBColor ambientColor{.025f, .025f, .025f};

	//--- FINAL COLOR ---
	RGBColor finalColor{ambientColor + specularColor + diffuseColor};
	finalColor.MaxToOne();
	m_pBackBufferPixels[static_cast<int>(v.vertex.Position.x + (v.vertex.Position.y * m_Width))] =
		GetSDL_ARGBColor(finalColor);
}

OutGoingVertex Renderer::CalcDepthVertex(const std::shared_ptr<Vertex>& vertex01,
                                         const std::shared_ptr<Vertex>& vertex02,
                                         const std::shared_ptr<Vertex>& vertex03, const float w0, const float w1,
                                         const float w2)
{
	OutGoingVertex v{};
	float v0InvDepth{1 / vertex01->screenPosition.w}, v1InvDepth{1 / vertex02->screenPosition.w}, v2InvDepth{
		      1 / vertex03->screenPosition.w
	      };

	v.depthInterpolated = 1.f / (v0InvDepth * w0 + v1InvDepth * w1 + v2InvDepth * w2);
	v.vertex.Uv = vertex01->vertex.Uv * w0 * v0InvDepth + vertex02->vertex.Uv * w1 * v1InvDepth + vertex03->vertex.Uv *
		w2 * v2InvDepth;
	v.vertex.Normal = vertex01->vertex.Normal * w0 * v0InvDepth + vertex02->vertex.Normal * w1 * v1InvDepth + vertex03->
		vertex.Normal * w2 * v2InvDepth;
	v.vertex.Tangent = vertex01->vertex.Tangent * w0 * v0InvDepth + vertex02->vertex.Tangent * w1 * v1InvDepth +
		vertex03->vertex.Tangent * w2 * v2InvDepth;
	v.viewDirection = vertex01->viewDirection * w0 * v0InvDepth + vertex02->viewDirection * w1 * v1InvDepth + vertex03->
		viewDirection * w2 * v2InvDepth;

	v.MultiplyDepthInterpolated();
	return v;
}
