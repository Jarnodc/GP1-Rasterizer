#pragma once
#include "BaseEffect.h"
#include <vector>
class Mesh final
{
public:
	struct Base_Vertex final
	{
		Elite::FPoint4 Position;
		Elite::RGBColor Color;
		Elite::FVector2 Uv;
		Elite::FVector3 Normal;
		Elite::FVector3 Tangent;
	};
	Mesh(ID3D11Device* pDevice, BaseEffect* pEffect, std::vector<Base_Vertex> vertices, const std::vector<uint32_t>& indices, const Elite::FVector3& position);
	~Mesh();

	void Render(ID3D11DeviceContext* pDeviceContext);
	void SetMaps(ID3D11ShaderResourceView* pDiffuse, ID3D11ShaderResourceView* pnormal, ID3D11ShaderResourceView* pspecular, ID3D11ShaderResourceView* pgloss);

	void ToggleSamplerState() const;
	void ToggleCullState() const;

	void SetWorldProjMat(float* mat) const { m_pEffect->SetWorldProjMat(mat); }
	void SetViewInvMat(float* mat) const { m_pEffect->SetViewInvMat(mat); }
	void SetWorldMat(float* mat) const { m_pEffect->SetWorldMat(mat); }

	BaseEffect* GetEffect() const{ return m_pEffect; }
	Elite::FVector3 GetPosition() const{ return m_Postion; }
	BaseEffect::CullMode GetCullMode() const{ return m_pEffect->GetCullMode(); }
private:
	const Elite::FVector3 m_Postion{ 0,0,0 };
	HRESULT CreateLayout(ID3D11Device* pDevice);
	HRESULT CreateBuffer(ID3D11Device* pDevice, const std::vector<Base_Vertex>& vertices, const std::vector<uint32_t>& indices);

	BaseEffect* m_pEffect = nullptr;

	ID3D11InputLayout* m_pLayout = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	uint32_t m_AmountIndices = 0;
};
