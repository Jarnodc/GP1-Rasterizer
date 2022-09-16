#include "pch.h"
#include "BaseEffect.h"

BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	:m_pEffect{ LoadEffect(pDevice, assetFile) }
{
	m_pTechniquefront = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechniquefront->IsValid())
	std::wcout << L"m_pTechniquefront is not valid\n";
	m_pTechniqueback = m_pEffect->GetTechniqueByName("DefaultTechniqueback");
	if (!m_pTechniqueback->IsValid())
		std::wcout << L"m_pTechniqueback is not valid\n";
	m_pTechniquenone = m_pEffect->GetTechniqueByName("DefaultTechniquenone");
		if (!m_pTechniquenone->IsValid())
			std::wcout << L"m_pTechniquenone is not valid\n";		

	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
	std::wcout << L"gWorldViewProj is not valid\n";
	
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	std::wcout << L"gDiffuseMap is not valid\n";
	
	m_pViewInverse = m_pEffect->GetVariableByName("gViewInvMatrix")->AsMatrix();
	if (!m_pViewInverse->IsValid())
	std::wcout << L"gViewInvMatrix is not valid\n";
	
	m_pWorldMat = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pWorldMat->IsValid())
	std::wcout << L"gWorldMatrix is not valid\n";

	auto sampler = m_pEffect->GetVariableByName("gSamplerState")->AsScalar();
	if (!sampler->IsValid())
		std::wcout << L"gSamplerState is not valid\n";
	else
		sampler->SetInt(0);
}

BaseEffect::~BaseEffect()
{
	m_pMatWorldViewProjVariable->Release();
	m_pDiffuseMapVariable->Release();
	m_pViewInverse->Release();
	m_pWorldMat->Release();
	m_pTechniquefront->Release();
	m_pTechniqueback->Release();
	m_pTechniquenone->Release();
	m_pEffect->Release();
}

ID3DX11Effect* BaseEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result = S_OK;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;
	DWORD shaderFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			char* pErrors = (char*)pErrorBlob->GetBufferPointer();

			std::wstringstream ss;

			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;
			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to createEffectFromFile!\nPath: " << assetFile; std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}
	return pEffect;
}

ID3DX11EffectTechnique* BaseEffect::GetTechnique() const
{
	switch (m_CS)
	{
	case CullMode::front:
		return m_pTechniqueback;
	case CullMode::back:
		return m_pTechniquefront;
	case CullMode::no:
		return m_pTechniquenone;
	default:
		return m_pTechniquefront;
	}
}

ID3DX11Effect* BaseEffect::GetEffect() const
{
	return m_pEffect;
}

void BaseEffect::SetWorldProjMat(float* mat)
{
	if (m_pMatWorldViewProjVariable->IsValid())
		m_pMatWorldViewProjVariable->SetMatrix(mat);
}

void BaseEffect::SetViewInvMat(float* mat)
{
	if (m_pViewInverse->IsValid())
		m_pViewInverse->SetMatrix(mat);
}

void BaseEffect::SetWorldMat(float* mat)
{
	if (m_pWorldMat->IsValid())
		m_pWorldMat->SetMatrix(mat);
}
void BaseEffect::ToggleSamplerState()
{
	auto sampler = m_pEffect->GetVariableByName("gSamplerState")->AsScalar();
	if (!sampler->IsValid())
		std::wcout << L"gSamplerState is not valid\n";
	else
	{
		std::string samp;
		switch (m_SS)
		{
		case gSamplerState::POINT:
			sampler->SetInt(0);
			m_SS = gSamplerState::LINEAR;
			samp = "Linear";
			break;
		case gSamplerState::LINEAR:
			sampler->SetInt(1);
			m_SS = gSamplerState::ANISOTROPIC;
			samp = "Anisotropic";
			break;
		case gSamplerState::ANISOTROPIC:
			sampler->SetInt(2);
			m_SS = gSamplerState::POINT;
			samp = "Point";
			break;
		}
		std::cout << "SamplerState changed to " << samp << std::endl;
	}
}
void BaseEffect::ToggleCullState()
{
	std::string cull;
	switch (m_CS)
	{
	case CullMode::back:
		m_CS = CullMode::front;
		cull = "Front-Face";
		break;
	case CullMode::front:
		m_CS = CullMode::no;
		cull = "No";
		break;
	case CullMode::no:
		m_CS = CullMode::back;
		cull = "Back-Face";
		break;
	}
	std::cout << "Cullstate changed to " << cull << " culling" << std::endl;
}