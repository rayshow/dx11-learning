
#include"base_define.hlsli"

cbuffer cbPerFrame : register(b0)
{
	float4x4 rotateProject;
}

SamplerState     sampleLinear             : register(s0);
TextureCube      specularEnv              : register(t0);

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

	float4 posPS = mul(float4(I.f3Position, 1), rotateProject);

	O.f3Dir = normalize(I.f3Position);

	//ndcλ��
	O.f4Position = posPS;

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
	O.color0 = specularEnv.SampleLevel(sampleLinear, I.f3Dir, 0);
	return O;
}
