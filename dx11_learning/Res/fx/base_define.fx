#ifndef BASE_DEFINE_HEADER__
#define BASE_DEFINE_HEADER__

#define PI     3.14159265358979323
#define INV_PI 0.31830988618379067
#define LN2	   0.69314718055994531
#define LN2H   0.34657359027997265

//后期处理PS参数
struct PS_FullScreenInput
{
	float4 f4Position   : SV_Position;
	float2 f2TexCoord   : TEXTURE;
};

// 普通模型的顶点参数
struct VS_Input_Xyznuv
{
	float3 f3Position    : POSITION;
	float3 f3Normal      : NORMAL;
	float2 f2TexCoord    : TEXCOORD0;
};

// 单片元单输出
struct PS_Output_Single
{
	float4 color0    : SV_Target0;
};




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







sampler PointSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = clamp;
	AddressV = clamp;
	MipLODBias = 0.0f;
};

SamplerState LinearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MipLODBias = 0.0f;
};

SamplerState anisotropicSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
	BorderColor = float4(0, 0, 0, 0);
};

SamplerState clampSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
	BorderColor = float4(0, 0, 0, 0);
};

/*

Texture2D                   Albedomap:AlbedoMap;
Texture2D                   Normalmap:NormalMap;
Texture2D                   Miscmap:MiscMap;
Texture2D                   Emitmap:EmitMap;
TextureCube                 Irridiancemap:IrridianceMap;
TextureCube                 FilteredSpecularmap: FilteredSpecularMap;
Texture2D                   IntergeLukupmap: IntergeLukupMap;
TextureCube                 EnvCubemap:EnvCubeMap;

*/
/*
float4x4 World : WorldMatrix;
float4x4 View: ViewMatrix;
float4x4 Project: ProjectMatrix;
float4x4 InvView: InvViewMatrix;
float4x4 InvProject : InvProjectMatrix;
float4x4 RotateProject: RotateProjectMatrix;
float4x4 WorldViewProject: WorldViewProjectMatrix;
float4   CamaraWorldPos: CamaraWorldPosition;

*/

#endif