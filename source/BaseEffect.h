#pragma once
#include <sstream>
class BaseEffect
{
public:
	//-------------------------------------------------
	// Cullmode enum class								
	//-------------------------------------------------
	enum class CullMode
	{
		front,
		back,
		no
	};
	BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~BaseEffect();

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	void SetWorldProjMat(float* mat);
	void SetViewInvMat(float* mat);
	void SetWorldMat(float* mat);

	ID3DX11EffectTechnique* GetTechnique() const;
	ID3DX11Effect* GetEffect() const;

	virtual void SetMaps(ID3D11ShaderResourceView* pDiffuse, ID3D11ShaderResourceView* pNormal, ID3D11ShaderResourceView* pSpecular, ID3D11ShaderResourceView* pGloss) = 0;
	void ToggleSamplerState();
	void ToggleCullState();
	CullMode GetCullMode() const { return m_CS; }
protected:
	//-------------------------------------------------
	// SamplerState enum class								
	//-------------------------------------------------
	enum class gSamplerState
	{
		POINT,
		LINEAR,
		ANISOTROPIC
	};
	ID3DX11Effect* m_pEffect = nullptr;
	ID3DX11EffectTechnique* m_pTechniquefront = nullptr;
	ID3DX11EffectTechnique* m_pTechniqueback = nullptr;
	ID3DX11EffectTechnique* m_pTechniquenone = nullptr;

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable = nullptr;

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable = nullptr;
	ID3DX11EffectMatrixVariable* m_pViewInverse = nullptr;
	ID3DX11EffectMatrixVariable* m_pWorldMat = nullptr;
	gSamplerState m_SS{ gSamplerState::POINT };
	CullMode m_CS{ CullMode::back };
};
