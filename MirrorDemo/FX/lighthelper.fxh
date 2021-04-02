struct SDirectionalLight
{
	float4 m_ambient;
	float4 m_diffuse;
	float4 m_specular;

	float3 m_direction;
	float m_padding;
};

struct SPointLight
{
	float4 m_ambient;
	float4 m_diffuse;
	float4 m_specular;

	float3 m_position;
	float m_range;

	float3 m_attenuation;
	float m_padding;
};

struct SSpotLight
{
	float4 m_ambient;
	float4 m_diffuse;
	float4 m_specular;

	float3 m_position;
	float m_range;

	float3 m_direction;
	float m_spotExp;

	float3 m_attenuation;
	float m_padding;
};

struct SMaterial
{
	float4 m_ambient;
	float4 m_diffuse;
	float4 m_specular;
	float4 m_reflect;
};

void computeDirectionalLight(
	SMaterial mat,
	SDirectionalLight l,
	float3 normal,
	float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 specular
)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = -l.m_direction;

	ambient = mat.m_ambient * l.m_ambient;

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		diffuse = diffuseFactor * mat.m_diffuse * l.m_diffuse;

		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.m_specular.w);
		specular = specFactor * mat.m_specular * l.m_specular;
	}
}

void computePointLight(
	SMaterial mat,
	SPointLight l,
	float3 pos,
	float3 normal,
	float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 specular
)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = l.m_position - pos;

	float d = length(lightVec);
	if (d > l.m_range)
	{
		return;
	}

	lightVec /= d;

	ambient = mat.m_ambient * l.m_ambient;

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		diffuse = diffuseFactor * mat.m_diffuse * l.m_diffuse;

		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.m_specular.w);
		specular = specFactor * mat.m_specular * l.m_specular;
	}

	float attenuation = 1.0f / dot(l.m_attenuation, float3(1.0f, d, d * d));
	diffuse *= attenuation;
	specular *= attenuation;
}

void computeSpotLight(
	SMaterial mat,
	SSpotLight l,
	float3 pos,
	float3 normal,
	float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 specular
)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = l.m_position - pos;

	float d = length(lightVec);
	if (d > l.m_range)
	{
		return;
	}

	lightVec /= d;

	ambient = mat.m_ambient * l.m_ambient;

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		diffuse = diffuseFactor * mat.m_diffuse * l.m_diffuse;

		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.m_specular.w);
		specular = specFactor * mat.m_specular * l.m_specular;
	}

	float spot = pow(max(dot(-lightVec, l.m_direction), 0.0f), l.m_spotExp);
	ambient *= spot;

	float attenuation = spot / dot(l.m_attenuation, float3(1.0f, d, d * d));
	diffuse *= attenuation;
	specular *= attenuation;
}
