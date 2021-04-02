#include "lighthelper.fxh"

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
	float2 m_tex : TEXCOORD;
};

struct SVertexOut
{
	float4 m_posH : SV_POSITION;
	float3 m_posW : POSITION;
	float3 m_normalW : NORMAL;
	float2 m_tex : TEXCOORD;
};

SVertexOut VS(SVertexIn vIn)
{
	SVertexOut vOut;

	vOut.m_posW = mul(float4(vIn.m_posL, 1.0f), g_world).xyz;
	vOut.m_normalW = mul(vIn.m_normalL, (float3x3)g_worldInvTranspose);

	vOut.m_posH = mul(float4(vIn.m_posL, 1.0f), g_worldViewProj);

	vOut.m_tex = mul(float4(vIn.m_tex, 0.0f, 1.0f), g_texTransform).xy;

	return vOut;
}

float4 PS(
	SVertexOut pIn,
	uniform int g_lightCount,
	uniform bool g_useTexture,
	uniform bool g_alphaClip,
	uniform bool g_fogEnabled
) : SV_Target
{
	pIn.m_normalW = normalize(pIn.m_normalW);

	float3 toEye = g_eyePosW - pIn.m_posW;

	float distToEye = length(toEye);

	toEye /= distToEye;

	float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if (g_useTexture)
	{
		texColor = g_diffuseMap.Sample(samAnisotropic, pIn.m_tex);

		if (g_alphaClip)
		{
			clip(texColor.a - 0.1f);
		}
	}

	float4 litColor = texColor;
	if (g_lightCount > 0)
	{
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

		litColor = texColor * (ambient + diffuse) + specular;
	}

	if (g_fogEnabled)
	{
		float fogLerp = saturate((distToEye - g_fogStart) / g_fogRange);

		litColor = lerp(litColor, g_fogColor, fogLerp);
	}

	litColor.a = g_material.m_diffuse.a * texColor.a;

	return litColor;
}

technique11 Light1
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(1, false, false, false)));
	}
}

technique11 Light2
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(2, false, false, false)));
	}
}

technique11 Light3
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, false)));
	}
}

technique11 Light0Tex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, false, false)));
	}
}

technique11 Light1Tex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(1, true, false, false)));
	}
}

technique11 Light2Tex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(2, true, false, false)));
	}
}

technique11 Light3Tex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(3, true, false, false)));
	}
}

technique11 Light0TexAlphaClip
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, false)));
	}
}

technique11 Light1TexAlphaClip
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(1, true, true, false)));
	}
}

technique11 Light2TexAlphaClip
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(2, true, true, false)));
	}
}

technique11 Light3TexAlphaClip
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, false)));
	}
}

technique11 Light1Fog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(1, false, false, true)));
	}
}

technique11 Light2Fog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(2, false, false, true)));
	}
}

technique11 Light3Fog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, true)));
	}
}

technique11 Light0TexFog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, false, true)));
	}
}

technique11 Light1TexFog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(1, true, false, true)));
	}
}

technique11 Light2TexFog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(2, true, false, true)));
	}
}

technique11 Light3TexFog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(3, true, false, true)));
	}
}

technique11 Light0TexAlphaClipFog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, true)));
	}
}

technique11 Light1TexAlphaClipFog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(1, true, true, true)));
	}
}

technique11 Light2TexAlphaClipFog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(2, true, true, true)));
	}
}

technique11 Light3TexAlphaClipFog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, true)));
	}
}
