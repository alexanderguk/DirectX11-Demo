cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 m_posL : POSITION;
	float4 m_color : COLOR;
};

struct VertexOut
{
	float4 m_posH : SV_POSITION;
	float4 m_color : COLOR;
};

VertexOut VS(VertexIn vIn)
{
	VertexOut vOut;
	vOut.m_posH = mul(float4(vIn.m_posL, 1.0f), gWorldViewProj);
	vOut.m_color = vIn.m_color;

	return vOut;
}

float4 PS(VertexOut pIn) : SV_Target
{
	return pIn.m_color;
}

technique11 ColorTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
