cbuffer cbPerObject : register(b0)
{
	float4x4 g_f4x4World;
	float4x4 g_f4x4View;
	float4x4 g_f4x4Project;
	float4x4 g_f4x4ConvolutionViews[6];
	float4   g_level;
}

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState                g_SamplePoint          : register(s0);
SamplerState                g_SampleBiLinear       : register(s1);
SamplerState                g_SampleTriLinear      : register(s1);

TextureCube					g_cube :                 register(t0);


#include"IBL.hlsl"

//后期处理PS参数
struct PS_FullScreenInput
{
	float4 f4Position   : SV_Position;
	float2 f2TexCoord   : TEXTURE;
};


// 普通模型的顶点参数
struct VS_NormalVertexLayout
{
	float3 f3Position    : POSITION;
	float3 f3Normal      : NORMAL;
	float2 f2TexCoord    : TEXCOORD0;
};


// 单片元单输出
struct PS_SingleOutput
{
	float4 rt0    : SV_Target0;
};

//普通模型的PS参数
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float2 f2TexCoord   : TEXTURE;
};


//普通模型的PS参数
struct GS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float2 f2TexCoord   : TEXTURE;
	uint RTIndex		: SV_RenderTargetArrayIndex;
};


//全屏处理公共顶点着色器
PS_FullScreenInput VS_FullScreenProcess(uint VertexID: SV_VertexID)
{
	PS_FullScreenInput O;
	O.f2TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	O.f4Position = float4(O.f2TexCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return O;
}


//顶点着色，普通模型
PS_TranslateInput VS_RenderNormalMesh(VS_NormalVertexLayout I)
{
	PS_TranslateInput O;
	float4 posMS = float4(I.f3Position, 1);

	float4 posWS = mul(posMS, g_f4x4World);
	float4 posVS = mul(posWS, g_f4x4ConvolutionViews[0]);

	//ndc位置
	O.f4Position = mul(posVS, g_f4x4Project);
	//normal 
	O.f3Normal = I.f3Normal;
	//coord
	O.f2TexCoord = I.f2TexCoord;

	return O;
}

//结果显示
PS_SingleOutput PS_ResultPass(PS_TranslateInput I)
{
	PS_SingleOutput O;
	float2 coord = I.f2TexCoord;
	float4 result = float4(0.1,0,0.5,1);
	O.rt0 = result;
	return O;
}



//顶点着色，普通模型
PS_TranslateInput VS_RenderCubemap(VS_NormalVertexLayout I)
{
	PS_TranslateInput O;
	float4 posMS = float4(I.f3Position, 1);
	O.f4Position = mul(posMS, g_f4x4World);
	O.f2TexCoord = I.f2TexCoord;
	O.f3Normal = I.f3Normal;
	return O;
}


//cubemap处理
[maxvertexcount(18)]
void GS_CubeMap(triangle PS_TranslateInput input[3], inout TriangleStream<GS_TranslateInput> CubeMapStream)
{
	for (int face = 0; face < 6; face++)
	{
		GS_TranslateInput output;
		output.RTIndex = face;
		for (int v = 0; v < 3; v++)
		{
			output.f3Normal = (input[v].f3Normal.xyz);
			output.f4Position = mul(input[v].f4Position, g_f4x4ConvolutionViews[face]);
			output.f4Position = mul(output.f4Position, g_f4x4Project);
			output.f2TexCoord = input[v].f2TexCoord;
			CubeMapStream.Append(output);
		}
		CubeMapStream.RestartStrip();
	}
}


//diffuse
PS_SingleOutput PS_Irridiancemap(GS_TranslateInput I)
{
	PS_SingleOutput O;
	float3 N = normalize(I.f3Normal);

	float3 color = ImportanceSampleDiffuse(g_cube, g_SampleTriLinear, N);
	O.rt0 = float4(color, 1);

	return O;
}



//diffuse
PS_SingleOutput PS_Specularmap(GS_TranslateInput I)
{
	PS_SingleOutput O;
	float3 N = normalize(I.f3Normal);

	float roughnessDelta = 1.0f / g_level.y;
	float roughness = g_level.x * roughnessDelta;
	float3 color = ImportanceSampleSpecular(g_cube, g_SampleTriLinear, N, roughness);

	O.rt0 = float4(color, 1);
	return O;
}
