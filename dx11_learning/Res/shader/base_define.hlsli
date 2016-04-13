#ifndef BASE_DEFINE
#define BASE_DEFINE


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


#endif