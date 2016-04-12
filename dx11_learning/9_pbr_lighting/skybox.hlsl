cbuffer cbPerFrame : register(b0)
{
	float4x4 g_f4x4World;
	float4x4 g_f4x4View;
	float4x4 g_f4x4Project;
}

SamplerState     g_SampleLinear          : register(s0);
TextureCube      g_Specular              : register(t0);

// 普通模型的顶点参数
struct VS_VertexLayout
{
	float3 f3Position    : POSITION;
	float3 f3Normal      : NORMAL;
	float2 f2TexCoord    : TEXCOORD;
};


//普通模型的PS参数
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float3 f3Dir		: COLOR2;
	float2 f2TexCoord   : TEXTURE;
};

// 单片元单输出
struct PS_SingleOutput
{
	float4 rt0    : SV_Target0;
};

//顶点着色，普通模型
PS_TranslateInput VS_FillBuffer(VS_VertexLayout I)
{
	PS_TranslateInput O;

	float3 posVS = mul(I.f3Position, (float3x3)g_f4x4View);

	O.f3Dir = normalize(I.f3Position);
	//ndc位置
	O.f4Position = mul(float4(posVS,1), g_f4x4Project);

	//normal 
	O.f3Normal = I.f3Normal;
	//coord
	O.f2TexCoord = I.f2TexCoord;

	return O;
}

//片元，普通模型
PS_SingleOutput PS_FillBuffer(PS_TranslateInput I)
{
	PS_SingleOutput O;
	float2 coord = I.f2TexCoord;

	float3 color = g_Specular.SampleLevel(g_SampleLinear, I.f3Dir, 0);
	O.rt0.rgb = color;
	return O;
}
