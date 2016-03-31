#define PI 3.14159265358979323f
#define INV_PI 0.31830988618379067239521257108191f

float4 sumDiffuse(float3 diffuseSample, float NoV, float4 result)
{
	result.xyz += diffuseSample;
	result.w++;
	return result;
}


float4
sumSpecular(float3 hdrPixel, float NoL, float4 result)
{
	result.xyz += (hdrPixel * NoL);
	result.w += NoL;
	return result;
}


//------------------------------------------------------------------------------------//
// Shader functions used by IblImportanceSamplingSpecular.fx                          //
//------------------------------------------------------------------------------------//
// D(h) for GGX.
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
float specularDSmith(float roughness, float NoH)
{
	float NoH2 = NoH * NoH;
	float r2 = roughness * roughness;
	return r2 / pow(NoH2 * (r2 - 1.0) + 1.0, 2.0);
}


//------------------------------------------------------------------------------------//
// Shader functions used by IblImportanceSamplingSpecular.fx                          //
//------------------------------------------------------------------------------------//
// D(h) for GGX.
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
float specularDSchlick(float roughness, float NoH)
{
	float r2 = roughness * roughness;
	float NoH2 = NoH * NoH;
	float a = 1.0 / (3.14159*r2*pow(NoH, 4));
	float b = exp((NoH2 - 1) / r2 * NoH2);
	return  a * b;
}



float GGXsmith(float NoV, float roughness)
{
	// http://graphicrants.blogspot.com.au/2013/08/specular-brdf-reference.html
	// Schlick-Beckmann G.
	// float k = roughness/2;
	// return NoV / (NoV * (1.0f - k) + k);

	// GGX G
	float r2 = pow(roughness, 2);
	return NoV * 2 / (NoV + sqrt((NoV*NoV) * (1.0f - r2) + r2));
}

float geometryForLutSmith(float roughness, float NoL)
{
	return GGXsmith(NoL, roughness * roughness);
}

// Visibility term
float visibilityForLutSmith(float roughness, float NoV)
{
	return GGXsmith(NoV, roughness * roughness);
}


// Fresnel Term.
// Inputs, view dot half angle.
float fresnelForLut(float VoH)
{
	return pow(1.0 - VoH, 5);
}


float GGXschlick(float NoV, float roughness)
{
	// http://graphicrants.blogspot.com.au/2013/08/specular-brdf-reference.html
	// Schlick-Beckmann G.
	// float k = roughness/2;
	// return NoV / (NoV * (1.0f - k) + k);

	// GGX G
	float r2 = pow(roughness, 2);
	return NoV * 2 / (NoV + sqrt((NoV*NoV) * (1.0f - r2) + r2));
}

float geometryForLutSchlick(float roughness, float NoL)
{
	return GGXschlick(NoL, roughness * roughness);
}

// Visibility term
float visibilityForLutSchlick(float roughness, float NoV)
{
	return GGXschlick(NoV, roughness * roughness);
}



float2 Hammersley(uint i, uint N)
{
	float ri = reversebits(i) * 2.3283064365386963e-10f;
	return float2(float(i) / float(N), ri);
}


float3 GGXSphereSample(float2 Xi, float3 N)
{
	float CosTheta = 1.0 - Xi.y;
	float SinTheta = sqrt(1.0 - CosTheta*CosTheta);
	float Phi = 2 * PI*Xi.x;

	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;

	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 importanceSampleGGX(float2 Xi, float roughness, float3 N)
{
	float a = roughness * roughness;
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
	float SinTheta = sqrt(1 - CosTheta * CosTheta);

	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;

	float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);
	return TangentX * H.x + TangentY * H.y + N * H.z;
}


// Summation of Lut term while iterating over samples
float2 sumLut(float2 current, float G, float V, float F, float VoH, float NoL, float NoH, float NoV)
{
	G = G * V;
	float G_Vis = G * VoH / (NoH * NoV);
	current.x += (1.0 - F) * G_Vis;
	current.y += F * G_Vis;

	return current;
}




// Compute Lod using inverse solid angle and pdf.
// From Chapter 20.4 Mipmap filtered samples in GPU Gems 3.
// http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
float3 ImportanceSampleDiffuse(TextureCube env, SamplerState envSampler, float3 N)
{
	const int ConvolutionSampleCount = 1024;
	float3 V = N;
	float4 result = float4(0, 0, 0, 0);
	float SampleStep = 1;
	uint sampleId = 0;
	uint cubeWidth, cubeHeight;
	env.GetDimensions(cubeWidth, cubeHeight);

	for (uint i = 0; i < ConvolutionSampleCount; i++)
	{
		float2 Xi = Hammersley(i, ConvolutionSampleCount);
		float3 H =  GGXSphereSample(Xi, N);
		float3 L =  normalize(2 * dot(V, H) * H - V);
		float NoL = saturate(dot(N, L));
		{
			float pdf = max(0.0, dot(N, L) * INV_PI);
			float solidAngleTexel = 4 * PI / (6 * cubeWidth * cubeWidth);
			float solidAngleSample = 1.0 / (ConvolutionSampleCount * pdf);
			float lod = 0.5 * log2((float)(solidAngleSample / solidAngleTexel));

			//float3 diffuseSample = rescaleHDR(ConvolutionSrc.SampleLevel(EnvMapSampler, H, lod).rgb);
			float3 diffuseSample = env.SampleLevel(envSampler, H, lod).rgb;
			result = sumDiffuse(diffuseSample, NoL, result);
		}
		sampleId += 1;
	}
	if (result.w == 0)
		return result.xyz;
	else
		return (result.xyz / result.w);
}


//
// Compute pdf of BRDF
// Taken from Epic's Siggraph 2013 Lecture:
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
//
float3 ImportanceSampleSpecular(TextureCube env, SamplerState envSampler, float3 N, float ConvolutionRoughness)
{
	const int ConvolutionSampleCount = 1024;
	float3 V = N;
	float4 result = float4(0, 0, 0, 0);

	float SampleStep = (1 / 1);
	uint sampleId = 0;

	uint cubeWidth, cubeHeight;
	env.GetDimensions(cubeWidth, cubeHeight);

	for (uint i = 0; i < ConvolutionSampleCount; i++)
	{
		float2 Xi = Hammersley(i, ConvolutionSampleCount);

		float3 H = importanceSampleGGX(Xi, ConvolutionRoughness, N);
		float3 L = 2 * dot(V, H) * H - V;
		float NoL = max(dot(N, L), 0);
		float VoL = max(dot(V, L), 0);
		float NoH = max(dot(N, H), 0);
		float VoH = max(dot(V, H), 0);
		if (NoL > 0)
		{
			float Dh = specularDSmith(ConvolutionRoughness, NoH);
			float pdf = Dh * NoH / (4 * VoH);
			float solidAngleTexel = 4 * PI / (6 * cubeWidth * cubeWidth);
			float solidAngleSample = 1.0 / (ConvolutionSampleCount * pdf);
			float lod = ConvolutionRoughness == 0 ? 0 : 0.5 * log2((float)(solidAngleSample / solidAngleTexel));
			float3 hdrPixel = env.SampleLevel(envSampler, L, lod).rgb;
			result = sumSpecular(hdrPixel, NoL, result);
		}
		sampleId += 1;
	}

	if (result.w == 0)
		return result.xyz;
	else
		return (result.xyz / result.w);
}

float3 ImportanceSampleSpecular2(TextureCube env, SamplerState envSampler, float3 N, float ConvolutionRoughness)
{
	const int ConvolutionSampleCount = 1024;
	float3 V = N;
	float4 result = float4(0, 0, 0, 0);

	float SampleStep = (1 / 1);
	uint sampleId = 0;

	uint cubeWidth, cubeHeight;
	env.GetDimensions(cubeWidth, cubeHeight);

	for (uint i = 0; i < ConvolutionSampleCount; i++)
	{
		float2 Xi = Hammersley(i, ConvolutionSampleCount);

		float3 H = importanceSampleGGX(Xi, ConvolutionRoughness, N);
		float3 L = 2 * dot(V, H) * H - V;
		float NoL = max(dot(N, L), 0);

		if (NoL > 0)
		{
			float3 hdrPixel = env.SampleLevel(envSampler, L, 0).rgb;
			result = sumSpecular(hdrPixel, NoL, result);
		}
		sampleId += 1;
	}

	if (result.w == 0)
		return result.xyz;
	else
		return (result.xyz / result.w);
}
