#pragma once
#include <sstream>
#include "BaseEffect.h"

class Effect : public BaseEffect
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~Effect() override;

	virtual void SetMaps(ID3D11ShaderResourceView* pDiffuse, ID3D11ShaderResourceView* pNormal, ID3D11ShaderResourceView* pSpecular, ID3D11ShaderResourceView* pGloss) override;
private:
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable = nullptr;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable = nullptr;
};
