cbuffer cbPerFrame : register(b0)
{
	float4x4 World;
	float4x4 WorldViewProject;
}

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

//��ͨģ�͵�PS����
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float2 f2TexCoord   : TEXTURE;
};

//������ɫ����ͨģ��
PS_TranslateInput VS_FillBuffer(VS_Input_Xyznuv I)
{
	PS_TranslateInput O;

	float4 posPS = mul(float4(I.f3Position, 1), WorldViewProject);

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
	O.color0 = float4(0.3, 0, 0, 1);
	return O;
}


technique11 Default
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_FillBuffer()));
		SetPixelShader(CompileShader(ps_5_0, PS_FillBuffer()));
	}
};