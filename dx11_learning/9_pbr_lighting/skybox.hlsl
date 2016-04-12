cbuffer cbPerFrame : register(b0)
{
	float4x4 g_f4x4World;
	float4x4 g_f4x4View;
	float4x4 g_f4x4Project;
}

SamplerState     g_SampleLinear          : register(s0);
TextureCube      g_Specular              : register(t0);

// ��ͨģ�͵Ķ������
struct VS_VertexLayout
{
	float3 f3Position    : POSITION;
	float3 f3Normal      : NORMAL;
	float2 f2TexCoord    : TEXCOORD;
};


//��ͨģ�͵�PS����
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float3 f3Dir		: COLOR2;
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

	float3 posVS = mul(I.f3Position, (float3x3)g_f4x4View);

	O.f3Dir = normalize(I.f3Position);
	//ndcλ��
	O.f4Position = mul(float4(posVS,1), g_f4x4Project);

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

	float3 color = g_Specular.SampleLevel(g_SampleLinear, I.f3Dir, 0);
	O.rt0.rgb = color;
	return O;
}
