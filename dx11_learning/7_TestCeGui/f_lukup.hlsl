#include"IBL.hlsl"

cbuffer  cbPerObject : register(b0)
{
	float4 Resolution;
}

RWTexture2D<float4> BRDFResult : register(u0);

float radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10f; // / 0x100000000
}

float2 hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), radicalInverse_VdC(i));
}


float2 integrate(float roughness, float NoV)
{
	float3 N = float3(0.0f, 0.0f, 1.0f);
	float3 V = float3(sqrt(1.0f - NoV * NoV), 0.0f, NoV);
	float2 result = float2(0, 0);

	const uint NumSamples = 1024;

	precise float Vis = visibilityForLutSmith(roughness, NoV);

	for (uint i = 0; i < NumSamples; i++)
	{
		float2 Xi = hammersley(i, NumSamples);
		float3 H = importanceSampleGGX(Xi, roughness, N);
		precise float3 L = 2.0f * dot(V, H) * H - V;

		float NoL = saturate(L.z);
		float NoH = saturate(H.z);
		float VoH = saturate(dot(V, H));
		float NoV = saturate(dot(N, V));
		if (NoL > 0)
		{
			precise float G = geometryForLutSmith(roughness, NoL);
			precise float F = fresnelForLut(VoH);
			result = sumLut(result, G, Vis, F, VoH, NoL, NoH, NoV);
		}
	}

	result.x = (result.x / float(NumSamples));
	result.y = (result.y / float(NumSamples));

	return result;
}



[numthreads(16, 16, 1)]
void CSMain(uint2 id : SV_DispatchThreadID)
{
	float roughness = (float)(id.y + 0.5) / Resolution.y;
	float NoV = (float)(id.x + 0.5) / Resolution.y;

	float2 result = integrate(roughness, NoV);
	BRDFResult[int2(id.x, id.y)] = float4(1, 1, 1, 1);// float4(result.x, result.y, roughness, 1);
}