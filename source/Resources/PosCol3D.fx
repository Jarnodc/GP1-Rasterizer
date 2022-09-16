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
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

//-----------------
//--- Variables ---
//-----------------
int gSamplerState : SS;

//----------------------------
//--- Input/Output Structs ---
//----------------------------
struct VS_INPUT
{
	float4 Position : POSITION;
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

	output.Position = mul(input.Position, gWorldMatrix);
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
float4 ShadePixel(VS_OUTPUT input,SamplerState samState) : SV_TARGET
{
	//--- Light Constants ---
	const float lightIntensity = 7.0f;
	const float3 lightDirection = { .577f, -.577f, -.577f };
	const float4 lightColor = { 1, 1, 1, 1.0f };

	//--- NORMAL MAPPING ---
	const float3 binormal = normalize(cross(input.Tangent, input.Normal));
	float3x3 tangentAxis;
	tangentAxis[0] = input.Tangent;
	tangentAxis[1] = binormal;
	tangentAxis[2] = input.Normal;

	const float4 normalMap = { gNormalMap.Sample(samState, input.Uv) };
	const float3 sampledValue = { 2.f * normalMap.r - 1.f, 2.f * normalMap.g - 1.f, 2.f * normalMap.b - 1.f };
	const float3 newNormal = { mul(sampledValue, tangentAxis) };

	//--- DIFFUSE COLOR ---
	float observedArea = saturate(dot(-newNormal, lightDirection));
	observedArea /= 3.14159265358979323846f;
	observedArea *= lightIntensity;
	const float4 diffuseColor = { gDiffuseMap.Sample(samState, input.Uv) * observedArea };
	
	//--- PHONG SPECULAR ---
	const float shininess = 25.f;
	const float3 reflection = reflect(-lightDirection, newNormal);
	float3 viewDir = normalize(input.Color - gViewInvMatrix[3].xyz);
	float specularStrength = saturate(dot(viewDir, reflection));

	const float4 glossMap = { gGlossinessMap.Sample(samState, input.Uv) };
	specularStrength = pow(specularStrength, glossMap.r * shininess);
	float4 specularColor = { gSpecularMap.Sample(samState, input.Uv) * specularStrength };
	normalize(specularColor);

	//--- AMBIENT COLOR ---
	const float4 ambientColor = { 0.025f, 0.025f, 0.025f, 1.0f };

	//--- FINAL COLOR ---
	const float4 finalColor = { saturate(ambientColor + specularColor + diffuseColor) };
	
	return finalColor;
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
};

//-------------------------
//--- DepthStencilState ---
//-------------------------
DepthStencilState gDepthStencilState
{
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