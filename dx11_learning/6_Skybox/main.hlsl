


cbuffer cbPerFrame : register(b0)
{
	float4x4 g_f4x4World;
	float4x4 g_f4x4View;
	float4x4 g_f4x4Project;
}
SamplerState     g_SampleLinear          : register(s0);

Texture2D        g_Diffuse               : register(t0);
Texture2D        g_Normal                : register(t1);
Texture2D        g_Specular              : register(t2);


// ��ͨģ�͵Ķ������
struct VS_VertexLayout
{
	float3 f3Position    : POSITION;
	float3 f3Normal      : NORMAL;
	float2 f2TexCoord    : TEXCOORD;
};


// NTBģ�͵Ķ������
struct VS_XYZNUVIIIWWTB
{
	float3 f3Position : POSITION;
	float3 f3Normal   : NORMAL;
	float2 f2TexCoord : TEXCOORD;
	float3 f3Tangent  : TANGENT;
	float3 f3Binormal : BINORMAL;
	float3 f3III      : III;
	float2 f2WW       : WW;
};



// ����NTBģ�͵�PS����
struct PS_TranslateInputNTB
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float3 f3Tangent    : COLOR2;
	float3 f3Binormal   : COLOR3;
	float2 f2TexCoord   : TEXTURE;
};

//��ͨģ�͵�PS����
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float2 f2TexCoord   : TEXTURE;
};

// ��ƬԪ�����
struct PS_SingleOutput
{
	float4 rt0    : SV_Target0;
};

//������ɫ����ͨģ��
PS_TranslateInput VS_FillBuffer(VS_VertexLayout I)
{
	PS_TranslateInput O;
	float4 posMS = float4(I.f3Position, 1);
	float4 posWS = mul(posMS, g_f4x4World);
	float4 posVS = mul(posWS, g_f4x4View);

	//ndcλ��
	O.f4Position = mul(posVS, g_f4x4Project);
	//normal 
	O.f3Normal = I.f3Normal;
	//coord
	O.f2TexCoord = I.f2TexCoord;

	return O;
}

//ƬԪ����ͨģ��
PS_SingleOutput PS_FillBuffer(PS_TranslateInput I)
{
	PS_SingleOutput O;
	float2 coord = I.f2TexCoord;

		float4 diffuse = g_Diffuse.SampleLevel(g_SampleLinear, coord, 0);
		float4 normal = g_Normal.SampleLevel(g_SampleLinear, coord, 0);
		float4 specular = g_Specular.SampleLevel(g_SampleLinear, coord, 0);

		clip(diffuse.a - 0.2);
		O.rt0.rgb =  diffuse;

	return O;
}
