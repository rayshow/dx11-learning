
#include"base_define.hlsli"

cbuffer cbPerFrame : register(b0)
{
	float4x4 viewProject;
}

SamplerState     sampleLinear            : register(s0);
TextureCube      g_Specular              : register(t0);

//��ͨģ�͵�PS����
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float3 f3Dir		: COLOR2;
	float2 f2TexCoord   : TEXTURE;
};

//������ɫ����ͨģ��
PS_TranslateInput VS_FillBuffer(VS_Input_Xyznuv I)
{
	PS_TranslateInput O;

	float3 posPS = mul(I.f3Position, (float3x3)viewProject);

	O.f3Dir = normalize(I.f3Position);

	//ndcλ��
	O.f4Position = float4(posPS, 1);

	//normal 
	O.f3Normal = I.f3Normal;
	//coord
	O.f2TexCoord = I.f2TexCoord;

	return O;
}

//ƬԪ����ͨģ��
PS_Output_Single PS_FillBuffer(PS_TranslateInput I)
{
	PS_Output_Single O;
	O.color0 = g_Specular.SampleLevel(sampleLinear, I.f3Dir, 0);
	return O;
}
