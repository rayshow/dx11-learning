#ifndef BASE_DEFINE
#define BASE_DEFINE


//���ڴ���PS����
struct PS_FullScreenInput
{
	float4 f4Position   : SV_Position;
	float2 f2TexCoord   : TEXTURE;
};


// ��ͨģ�͵Ķ������
struct VS_Input_Xyznuv
{
	float3 f3Position    : POSITION;
	float3 f3Normal      : NORMAL;
	float2 f2TexCoord    : TEXCOORD0;
};

// ��ƬԪ�����
struct PS_Output_Single
{
	float4 color0    : SV_Target0;
};


#endif