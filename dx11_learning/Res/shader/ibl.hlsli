#ifndef IBL_HEADER__
#define IBL_HEADER__

#include"brdf.hlsli"


uint ReverseBits32(uint bits)
{
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
	bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
	bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
	bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
	return bits;
}


// Reference: Real Shading in Unreal Engine 4
// by Brian Karis
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float3 sample_to_world(in float phi, in float cos_theta, in float sin_theta, in float3 N)
{
	float3 H;

	H.x = sin_theta * cos(phi);
	H.y = sin_theta * sin(phi);
	H.z = cos_theta;

	float3 up_vec = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 tangent_x = cross(up_vec, N);
	float3 tangent_y = cross(N, tangent_x);

	return tangent_x * H.x + tangent_y * H.y + N * H.z;
}


float2 Hammersley(uint Index, uint NumSamples, uint2 Random)
{
	float E1 = frac((float)Index / NumSamples + float(Random.x & 0xffff) / (1 << 16));
	float E2 = float(ReverseBits32(Index) ^ Random.y) * 2.3283064365386963e-10;
	return float2(E1, E2);
}


// Importance sample a GGX specular function
float3 ImportanceSampleGGX(float2 xi, float roughness, float3 N)
{
	float a = roughness*roughness;// pow(roughness + 1, 2);

	float phi = 2 * PI * xi.x;
	float cos_theta = sqrt((1 - xi.y) / (1 + (a*a - 1) * xi.y));
	float sin_theta = sqrt(1 - cos_theta * cos_theta);

	return sample_to_world(phi, cos_theta, sin_theta, N);
}



float3 PrefilterSpecularMap(
	float roughness,
	float3 R,
	uint numSamples,
	in TextureCube envMap,
	in SamplerState envSampler)
{
	float3 N = R;
	float3 V = R;
	float3 PrefilteredColor = 0;
	float TotalWeight = 0;
	for (uint i = 0; i < numSamples; i++)
	{
		float2 Xi = Hammersley(i, numSamples, 0);
		float3 H = ImportanceSampleGGX(Xi, roughness, N);

		float3 L = 2 * dot(V, H) * H - V;
		float NoL = saturate(dot(N, L));
		if (NoL > 0)
		{
			float NoH = saturate(dot(N, H));
			float VoH = saturate(dot(V, H));
			float pdf = SpecularD(roughness, NoH)*NoH / (4 * VoH);
			float fOmegaS = 1.0 / (numSamples*pdf);
			float fOmegaP = 4.0 * PI / (6.0 * 256 * 256);
			float fMipBias = 0.0f;

			float fMipLevel = clamp(0.5 * log2(fOmegaS / fOmegaP), 0, 9);

			PrefilteredColor += envMap.SampleLevel(envSampler, L, fMipLevel).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
}







#endif

