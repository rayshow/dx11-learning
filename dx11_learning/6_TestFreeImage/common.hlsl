#ifndef COMMON_FUNC_CONST_HEADER__
#define COMMON_FUNC_CONST_HEADER__

#define PI   3.1415926535897932384626433832795
#define LN2  0.69314718055994530941723212145818
#define LN2H 0.34657359027997265470861606072909

float squr(float x)
{
	return x*x;
}

float pow5(float a)
{
	float a2 = a*a;
	return a2*a2*a;
}

float2 clipToScreenXY(float2 clip)
{
	return clip*float2(0.5, -0.5) + float2(0.5, 0.5);
}

float2 screenToClipXY(float2 screen)
{
	return (screen - float2(0.5, 0.5)) * float2(2, -2);
}

float3 clipToCamara(float3 v, float4x4 invProj)
{
	float4 vcs = mul(float4(v, 1), invProj);
	return vcs.xyz / vcs.w;
}

float3 screenToCamara(float2 coord, float clipZ, float4x4 invProj)
{
	float4 vcs = mul(float4(screenToClipXY(coord), clipZ, 1), invProj);
		return vcs.xyz / vcs.w;
}


float2 uvToClip(float2 uv)
{
	return (uv - float2(0.5, 0.5))*float2(2, -2);
}


float linearToClip(float linearZ, float A, float B)
{
	return A - B / linearZ;
}



float clipToLinear(float clipZ, float A, float B)
{
	return B / (A - clipZ);
}

float floatEqual(
	float a,
	float b)
{
	return (abs(a - b) < 0.00001f ? 1.0 : 0.0f);
}

#endif