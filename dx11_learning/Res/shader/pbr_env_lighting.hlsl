
cbuffer cbPerFrame : register(b0)
{
	float4x4 g_f4x4World;
	float4x4 g_f4x4View;
	float4x4 g_f4x4Project;
}


SamplerState     g_SampleLinear          : register(s0);

Texture2D        g_Albedo                : register(t0);
Texture2D        g_Normal                : register(t1);
Texture2D        g_Specular              : register(t2);


// 顶点参数
struct VS_VertexLayoutNTB
{
	float3 f3Position    : POSITION;
	float3 f3Normal      : NORMAL;
	float2 f2TexCoord    : TEXCOORD0;
	float3 f3Tangent	 : TANGENT;
	float3 f3Binormal	 : BINORMAL;
};

// PS参数
struct PS_TranslateInputNTB
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float3 f3Tangent    : COLOR2;
	float3 f3Binormal   : COLOR3;
	float2 f2TexCoord   : TEXTURE;
};


// 单片元单输出
struct PS_SingleOutput
{
	float4 rt0    : SV_Target0;
};

//顶点着色，普通模型
PS_TranslateInputNTB VS_FillBuffer(VS_VertexLayoutNTB I)
{
	PS_TranslateInputNTB O;
	float4 posMS = float4(I.f3Position, 1);
	float4 posWS = mul(posMS, g_f4x4World);
	float4 posVS = mul(posWS, g_f4x4View);

	//ndc位置
	O.f4Position = mul(posVS, g_f4x4Project);
	//normal 
	O.f3Normal = I.f3Normal;
	//coord
	O.f2TexCoord = I.f2TexCoord;

	return O;
}

//片元，普通模型
PS_SingleOutput PS_FillBuffer(PS_TranslateInputNTB I)
{
	PS_SingleOutput O;
	float2 coord = I.f2TexCoord;

	O.rt0.rgb = float4(0.2, 0, 0, 1);
	return O;
}
