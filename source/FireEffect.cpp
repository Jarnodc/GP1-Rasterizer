#include "pch.h"
#include "FireEffect.h"

FireEffect::FireEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	:BaseEffect(pDevice,assetFile)
{
}

FireEffect::~FireEffect()
{
}

void FireEffect::SetMaps(ID3D11ShaderResourceView* pDiffuse, ID3D11ShaderResourceView* pNormal, ID3D11ShaderResourceView* pSpecular, ID3D11ShaderResourceView* pGloss)
{
	if(m_pDiffuseMapVariable->IsValid())
		m_pDiffuseMapVariable->SetResource(pDiffuse);
}
