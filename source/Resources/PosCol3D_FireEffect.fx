//----------------
//--- Matricss ---
//----------------
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gViewInvMatrix : ViewInverseMatrix;

//----------------
//--- Textures ---
//----------------
Texture2D gDiffuseMap : DiffuseMap;

//-----------------
//--- Variables ---
//-----------------
int gSamplerState : SS;

//----------------------------
//--- Input/Output Structs ---
//----------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float2 Uv : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
	float2 Uv : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

SamplerState samplerPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Border;// or Mirror or Clamp or Border
	AddressV = Clamp;// or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};
SamplerState sampleLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border;// or Mirror or Clamp or Border
	AddressV = Clamp;// or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};
SamplerState samplerAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Border;// or Mirror or Clamp or Border
	AddressV = Clamp;// or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

//---------------------
//--- Vertex Shader ---
//---------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Position = mul(float4(input.Position, 1.f), gWorldMatrix);
	output.Position = mul(output.Position, gWorldViewProj);

	output.Color = (float3)mul(float4(input.Color, 1.f), gWorldMatrix);

	output.Normal = mul(input.Normal, (float3x3)gWorldMatrix);
	output.Tangent = mul(input.Tangent, (float3x3)gWorldMatrix); 

	output.Uv = input.Uv;

	return output;
}
//--------------------
//--- Pixel Shader ---
//--------------------
float4 ShadePixel(VS_OUTPUT input, SamplerState samState) : SV_TARGET
{
	return gDiffuseMap.Sample(samState, input.Uv);
}
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	switch (gSamplerState)
	{
	case 0:
		return ShadePixel(input, samplerPoint);
	case 1:
		return ShadePixel(input, sampleLinear);
	case 2:
		return ShadePixel(input, samplerAnisotropic);
	default:
		return ShadePixel(input, samplerPoint);
	}
}
//------------------
//--- BlendState ---
//------------------
BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	blendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

//-------------------------
//--- DepthStencilState ---
//-------------------------
DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;

	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail= keep;
	BackFaceStencilDepthFail= keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

//------------------------
//--- Rasterizer State ---
//------------------------
RasterizerState gRasterizerStateBack
{
	CullMode = back;
	FrontCounterClockwise = false;
};
RasterizerState gRasterizerStateNone
{
	CullMode = none;
	FrontCounterClockwise = false;
};
RasterizerState gRasterizerState
{
	CullMode = front;
	FrontCounterClockwise = false;
};

//-----------------
//--- Technique ---
//-----------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
};
technique11 DefaultTechniqueback
{
	pass P0
	{
		SetRasterizerState(gRasterizerStateBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
};
technique11 DefaultTechniquenone
{
	pass P0
	{
		SetRasterizerState(gRasterizerStateNone);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
};