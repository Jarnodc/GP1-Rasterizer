#include "pch.h"
#include "Effect.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
	:BaseEffect(pDevice, assetFile)
{
	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
		std::wcout << L"gNormalMap is not valid\n";

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
		std::wcout << L"gSpecularMap is not valid\n";

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
		std::wcout << L"gGlossinessMap is not valid\n";	
}

Effect::~Effect()
{
	m_pNormalMapVariable->Release();
	m_pSpecularMapVariable->Release();
	m_pGlossinessMapVariable->Release();
}

void Effect::SetMaps(ID3D11ShaderResourceView* pDiffuse, ID3D11ShaderResourceView* pNormal, ID3D11ShaderResourceView* pSpecular, ID3D11ShaderResourceView* pGloss)
{
	if (m_pDiffuseMapVariable->IsValid())
		m_pDiffuseMapVariable->SetResource(pDiffuse);

	if (m_pNormalMapVariable->IsValid())
		m_pNormalMapVariable->SetResource(pNormal);

	if (m_pSpecularMapVariable->IsValid())
		m_pSpecularMapVariable->SetResource(pSpecular);

	if (m_pGlossinessMapVariable->IsValid())
		m_pGlossinessMapVariable->SetResource(pGloss);
}

