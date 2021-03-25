#include "lighthelper.h"

cbuffer cbPerFrame
{
	SDirectionalLight g_dirLight;
	SPointLight g_pointLight;
	SSpotLight g_spotLight;
	float3 g_eyePosW;
};

cbuffer cbPerObject
{
	float4x4 g_world;
	float4x4 g_worldInvTranspose;
	float4x4 g_worldViewProj;
	SMaterial g_material;
};

struct SVertexIn
{
	float3 m_posL : POSITION;
	float3 m_normalL : NORMAL;
};

struct SVertexOut
{
	float4 m_posH : SV_POSITION;
	float3 m_posW : POSITION;
	float3 m_normalW : NORMAL;
};

SVertexOut VS(SVertexIn vIn)
{
	SVertexOut vOut;
	vOut.m_posW = mul(float4(vIn.m_posL, 1.0f), g_world).xyz;
	vOut.m_normalW = mul(vIn.m_normalL, (float3x3)g_worldInvTranspose);
	vOut.m_posH = mul(float4(vIn.m_posL, 1.0f), g_worldViewProj);

	return vOut;
}

float4 PS(SVertexOut pIn) : SV_Target
{
	pIn.m_normalW = normalize(pIn.m_normalW);

	float3 toEyeW = normalize(g_eyePosW - pIn.m_posW);

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 A, D, S;

	computeDirectionalLight(
		g_material, g_dirLight, pIn.m_normalW, toEyeW, A, D, S
	);
	ambient += A;
	diffuse += D;
	specular += S;

	computePointLight(
		g_material, g_pointLight, pIn.m_posW, pIn.m_normalW, toEyeW, A, D, S
	);
	ambient += A;
	diffuse += D;
	specular += S;

	computeSpotLight(
		g_material, g_spotLight, pIn.m_posW, pIn.m_normalW, toEyeW, A, D, S
	);
	ambient += A;
	diffuse += D;
	specular += S;

	float4 litColor = ambient + diffuse + specular;

	litColor.a = g_material.m_diffuse.a;

	return litColor;
}

technique11 LightTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
