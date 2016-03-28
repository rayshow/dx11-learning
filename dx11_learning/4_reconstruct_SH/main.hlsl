#include"common.hlsl"

cbuffer cbPerObject : register(b0)
{
	float4x4 g_f4x4World;
	float4x4 g_f4x4View;
	float4x4 g_f4x4Project;
	float4x4 colorConvent[3];
}

cbuffer cbEnvMap
{
	float4x4 g_invProjView;
	float4   g_camaraEye;
};

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState                g_SampleTrilinear      : register(s0);
TextureCube					env :					 register(t0);
Texture2D                   colorBuffer :                  register(t1);
Texture2D                   depthBuffer :                  register(t2);

struct Coeffs
{
	float3 val[9];
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

// 单片元单输出
struct PS_DoubleOutput
{
	float4 rt0    : SV_Target0;
	float4 rt1    : SV_Target1;
};

//普通模型的PS参数
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float2 f2TexCoord   : TEXTURE;
};




//顶点着色，普通模型
PS_TranslateInput VS_RenderCommonMesh(VS_NormalVertexLayout I)
{
	PS_TranslateInput O;
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

//全屏处理公共顶点着色器
PS_TranslateInput VS_FullScreenProcess(uint VertexID: SV_VertexID)
{
	PS_TranslateInput O;
	O.f2TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	O.f4Position = float4(O.f2TexCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return O;
}

PS_SingleOutput PS_Display(PS_TranslateInput I)
{
	PS_SingleOutput O;
	float2 coord = I.f2TexCoord;
	float3 depth = depthBuffer.SampleLevel(g_SampleTrilinear, coord, 0);
	float3 color = colorBuffer.SampleLevel(g_SampleTrilinear, coord, 0);




	if (depth.x == 0)
		O.rt0.rgb = float3(1, 0, 0);
	else
		O.rt0.rgb = color;
	return O;
}


void SH_Y2(float3 n, inout float sh[9])
{
	sh[0] = 0.282094791774f;
	sh[1] = -0.488602511903f * n.y;
	sh[2] = 0.488602511903f * n.z;
	sh[3] =- 0.488602511903f * n.x;
	sh[4] = 1.09254843059f * n.x * n.y;
	sh[5] = -1.09254843059f * n.y * n.z;
	sh[6] = 0.315391565253f * (3 * n.z * n.z - 1);
	sh[7] = -1.09254843059f * n.x * n.z;
	sh[8] = 0.546274215296f * (n.x * n.x - n.y * n.y);
}

void SHResult(inout Coeffs c)
{
	c.val[0] = float3(3.629553, 2.629230, 2.330241);
	c.val[1] = float3(-1.758101, -1.436402, -1.259139);
	c.val[2] = float3(-0.248311, -0.101627, -0.010648);
	c.val[3] = float3(-0.345998, -0.223130, -0.101090);
	c.val[4] = float3(0.202062, 0.146508, 0.045813);
	c.val[5] = float3(0.466751, 0.253007, 0.116325);
	c.val[6] = float3(-0.901142, -0.762130, -0.741006);
	c.val[7] = float3(-0.044793, -0.033679, 0.004262);
	c.val[8] = float3(-0.821333, -0.322317, 0.033945);
}

float3 convolve(float sh[9], Coeffs result)
{
	float3 color = 0;
	for (int i = 0; i < 9; ++i)
	{
		float3 val = result.val[i] * sh[i];
		color += val;
	}
	return color;
}

//结果显示
PS_DoubleOutput PS_FillBufferPass(PS_TranslateInput I)
{
	PS_DoubleOutput O;
	float4 normal = float4(normalize(I.f3Normal), 1);
	float2 coord = I.f2TexCoord;
	float sh[9];
	Coeffs result;
	SH_Y2(normal.xyz, sh);
	SHResult(result);
	float3 color = convolve(sh, result);

	O.rt0.rgb = color;
	O.rt1.r = I.f4Position.z / I.f4Position.w;

	return O;
}




float3 filmicToneMapping(float3 color)
{
	//color = filmicToneMapping(color);
	//color = pow(color, 1 / 2.2);
	//if (color.x >1 || color.y > 1 || color.z > 1)
	//	color = float3(1, 0, 0);
	color = max(0, color - 0.004);
	color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
	return color;
}


