#include "lighthelper.fx"

cbuffer cbPerFrame
{
	SDirectionalLight g_dirLights[3];
	float3 g_eyePosW;

	float  g_fogStart;
	float  g_fogRange;
	float4 g_fogColor;
};

cbuffer cbPerObject
{
	float4x4 g_world;
	float4x4 g_worldInvTranspose;
	float4x4 g_worldViewProj;
	float4x4 g_texTransform;
	SMaterial g_material;
};

Texture2D g_diffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
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

float4 PS(SVertexOut pIn, uniform int g_lightCount) : SV_Target
{
	pIn.m_normalW = normalize(pIn.m_normalW);

	float3 toEye = g_eyePosW - pIn.m_posW;

	float distToEye = length(toEye);

	toEye /= distToEye;

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	[unroll]
	for (int i = 0; i < g_lightCount; ++i)
	{
		float4 A, D, S;
		computeDirectionalLight(
			g_material, g_dirLights[i], pIn.m_normalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		specular += S;
	}

	float4 litColor = ambient + diffuse + specular;
	litColor.a = g_material.m_diffuse.a;

	return litColor;
}

technique11 Light1
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(1)));
	}
}

technique11 Light2
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(2)));
	}
}

technique11 Light3
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(3)));
	}
}
