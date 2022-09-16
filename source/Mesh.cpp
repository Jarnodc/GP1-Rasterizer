#include "pch.h"
#include "Mesh.h"

Mesh::Mesh(ID3D11Device* pDevice, BaseEffect* pEffect, std::vector<Base_Vertex> vertices, const std::vector<uint32_t>& indices, const Elite::FVector3& position)
	:m_Postion{position}
	,m_pEffect{ pEffect }
{
	if (CreateLayout(pDevice) == S_OK)
		if (CreateBuffer(pDevice, vertices, indices) == S_OK)
			std::cout << "Buffer and Layout\n";
		else
			std::cout << "Failed to initializeBuffer\n";
	else
		std::cout << "Failed to initializeLayout\n";
}

Mesh::~Mesh()
{
	m_pLayout->Release();
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
	delete m_pEffect;
	m_pEffect = nullptr;
}
void Mesh::Render(ID3D11DeviceContext* pDeviceContext)
{
	// Set vertex buffer
	UINT stride = sizeof(Base_Vertex);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	// Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// Set the input layout
	pDeviceContext->IASetInputLayout(m_pLayout),
	// Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Render a triangle //change to toggle between point, anistostropic, ...

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pEffect->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
	}
}
void Mesh::SetMaps(ID3D11ShaderResourceView* pResourceView, ID3D11ShaderResourceView* pnormal, ID3D11ShaderResourceView* pspecular, ID3D11ShaderResourceView* pgloss)
{
	m_pEffect->SetMaps(pResourceView, pnormal, pspecular, pgloss);
}
void Mesh::ToggleSamplerState() const
{
	m_pEffect->ToggleSamplerState();
}
void Mesh::ToggleCullState() const
{
	m_pEffect->ToggleCullState();
}
HRESULT Mesh::CreateBuffer(ID3D11Device* pDevice, const std::vector<Base_Vertex>& vertices, const std::vector<uint32_t>& indices)
{

	HRESULT result = S_OK;

	// Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Base_Vertex) * (uint32_t)vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = vertices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return result;

	//Create index buffer
	m_AmountIndices = (uint32_t)indices.size();
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

	return result;
}
HRESULT Mesh::CreateLayout(ID3D11Device* pDevice)
{
	HRESULT result = S_OK;

	//Create Vertex Layout
	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};
	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 16;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 28;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 36;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 48;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pLayout);

	return result;
}
