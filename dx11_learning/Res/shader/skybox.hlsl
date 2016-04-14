
#include"base_define.hlsli"

cbuffer cbPerFrame : register(b0)
{
	float4x4 viewProject;
}

SamplerState     sampleLinear            : register(s0);
TextureCube      g_Specular              : register(t0);

//普通模型的PS参数
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float3 f3Dir		: COLOR2;
	float2 f2TexCoord   : TEXTURE;
};

//顶点着色，普通模型
PS_TranslateInput VS_FillBuffer(VS_Input_Xyznuv I)
{
	PS_TranslateInput O;

	float3 posPS = mul(I.f3Position, (float3x3)viewProject);

	O.f3Dir = normalize(I.f3Position);

	//ndc位置
	O.f4Position = float4(posPS, 1);

	//normal 
	O.f3Normal = I.f3Normal;
	//coord
	O.f2TexCoord = I.f2TexCoord;

	return O;
}

//片元，普通模型
PS_Output_Single PS_FillBuffer(PS_TranslateInput I)
{
	PS_Output_Single O;
	O.color0 = g_Specular.SampleLevel(sampleLinear, I.f3Dir, 0);
	return O;
}
