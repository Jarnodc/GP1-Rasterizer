#pragma once
#include <sstream>
#include "BaseEffect.h"

class FireEffect : public BaseEffect
{
public:
	FireEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~FireEffect() override;

	virtual void SetMaps(ID3D11ShaderResourceView* pDiffuse, ID3D11ShaderResourceView* pNormal, ID3D11ShaderResourceView* pSpecular, ID3D11ShaderResourceView* pGloss) override;
private:
};
