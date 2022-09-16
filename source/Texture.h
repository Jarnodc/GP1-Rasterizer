#pragma once
#include <SDL_image.h>
class Texture final
{
public:
	Texture(const char* path, ID3D11Device* m_pDevice);		// Constructor
	~Texture();																					// Destructor

	ID3D11ShaderResourceView* GetTextureResourceView() const { return m_pTextureResourceView; }
private:
	HRESULT CreateTexture(ID3D11Device* pDevice);

	SDL_Surface* m_pSurface{};
	ID3D11Texture2D* m_pTexture = nullptr;
	ID3D11ShaderResourceView* m_pTextureResourceView = nullptr;
};
