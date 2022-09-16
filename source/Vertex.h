#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "EMath.h"
#include "ERGBColor.h"
#include <SDL_image.h>

using namespace Elite;

//-----------------------------------------------------
// BoundingBox Struct									
//-----------------------------------------------------
struct BoundingBox final
{
	BoundingBox() = default;											// Constructor
	BoundingBox(const FPoint2& _topLeft, const FPoint2& _rightBottom)	// Constructor
		:topLeft(_topLeft), rightBottom(_rightBottom) {}

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	FPoint2 topLeft {};
	FPoint2 rightBottom {};
};

//-----------------------------------------------------
// Vertex Struct									
//-----------------------------------------------------
struct Vertex final
{
	Vertex() = default;						// Constructor
	Vertex(const Mesh::Base_Vertex& bvertex)			// Constructor
		:vertex{ bvertex }
	{
	}

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	Mesh::Base_Vertex vertex{};
	FPoint4 screenPosition{};
	FVector3 viewDirection = {};
};

//-----------------------------------------------------
// OutGoingVertex Struct									
//-----------------------------------------------------
struct OutGoingVertex final
{
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	Mesh::Base_Vertex vertex{};
	FVector3 viewDirection = {};
	float depthInterpolated = {};

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void MultiplyDepthInterpolated()
	{
		vertex.Uv *= depthInterpolated;
		vertex.Normal *= depthInterpolated;
		vertex.Tangent *= depthInterpolated;
		viewDirection *= depthInterpolated;
	}
};

//-----------------------------------------------------
// Texture Class									
//-----------------------------------------------------
class Texture_Rasterizer final
{
public:
	Texture_Rasterizer(const char* path)			// Constructor
		:pTexture{IMG_Load(path)} {}
	~Texture_Rasterizer()							// Destructor
	{
		SDL_FreeSurface(pTexture);
	}

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	RGBColor Sample(const FVector2& uv)
	{
		RGBColor returnColor{};

		//--- Get TextureSize ---
		const int textureWidth = pTexture->w;
		const int textureHeight = pTexture->h;

		//--- Get x and y position on texture ---
		const uint32_t* pixels = static_cast<uint32_t*>(pTexture->pixels);
		const int xTex = int(uv.x * textureWidth + 0.5f);
		const int yTex = int(uv.y * textureHeight + 0.5f);

		if (xTex < 0.f || xTex > textureWidth || yTex < 0.f || yTex > textureHeight)
			return returnColor;

		uint8_t rC, gC, bC = 0;
		SDL_GetRGB(pixels[xTex + (yTex * textureWidth)], pTexture->format, &rC, &gC, &bC);
		returnColor = RGBColor{ rC / 255.f, gC / 255.f, bC / 255.f };
		return returnColor;
	}
private:
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	SDL_Surface* pTexture{ nullptr };
};