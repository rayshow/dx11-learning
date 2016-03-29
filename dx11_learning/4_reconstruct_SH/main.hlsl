#include"common.hlsl"

cbuffer cbPerObject : register(b0)
{
	float4x4 g_f4x4World;
	float4x4 g_f4x4View;
	float4x4 g_f4x4Project;
	float4x4 colorConvent[3];
}

cbuffer cbEnvMap: register(b1)
{
	float4x4 g_invView;
	float4x4 g_invProj;
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
	float  depth = depthBuffer.SampleLevel(g_SampleTrilinear, coord, 0).r;
	float3 color = colorBuffer.SampleLevel(g_SampleTrilinear, coord, 0);

	if (depth == 0)
	{
		float3 posView = screenToCamara(coord, 1, g_invProj);
		float3 posWorld = mul(posView, (float3x3)g_invView);
		float3 eyeToPoint = normalize(g_camaraEye - posWorld);
		eyeToPoint.y = -eyeToPoint.y;
		color = env.SampleLevel(g_SampleTrilinear, eyeToPoint, 0);
	}

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

void SHResult_stpeters_cross(inout Coeffs c)
{
	c.val[0] = float3(3.627909, 2.628220, 2.329475);
	c.val[1] = float3(-1.764583, -1.440238, -1.262240);
	c.val[2] = float3(-0.245652, -0.099981, -0.009327);
	c.val[3] = float3(-0.345937, -0.223090, -0.101011);
	c.val[4] = float3(0.201632, 0.146153, 0.045499);
	c.val[5] = float3(0.471821, 0.255883, 0.118612);
	c.val[6] = float3(-0.900894, -0.761977, -0.740918);
	c.val[7] = float3(-0.047082, -0.035095, 0.003084);
	c.val[8] = float3(-0.820679, -0.321894, 0.034262);
}

void SHResult_uffizi_cross(inout Coeffs c)
{
	c.val[0] = float3(3.176455, 3.083038, 3.506252);
	c.val[1] = float3(-3.708286, -3.678735, -4.274433);
	c.val[2] = float3(-0.026662, -0.024183, -0.030795);
	c.val[3] = float3(0.081442, 0.084279, 0.113477);
	c.val[4] = float3(-0.186518, -0.188335, -0.246335);
	c.val[5] = float3(0.094862, 0.087919, 0.101911);
	c.val[6] = float3(-2.790150, -2.753834, -3.186087);
	c.val[7] = float3(-0.005642, -0.003932, -0.002459);
	c.val[8] = float3(-2.413705, -2.405697, -2.838634);
}


void SHResult_rnl_cross(inout Coeffs c)
{
	c.val[0] = float3(3.793014, 4.273633, 4.521361);
	c.val[1] = float3(-2.892342, -3.593026, -4.153489);
	c.val[2] = float3(0.389177, 0.305846, 0.108674);
	c.val[3] = float3(1.035336, 1.033241, 0.885729);
	c.val[4] = float3(-0.618883, -0.551082, -0.393519);
	c.val[5] = float3(-0.079163, 0.146668, 0.469398);
	c.val[6] = float3(-0.934035, -1.254196, -1.528312);
	c.val[7] = float3(0.582851, 0.513954, 0.375490);
	c.val[8] = float3(0.208647, -0.038505, -0.446823);
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
	SHResult_stpeters_cross(result);
	float3 color = convolve(sh, result);

	O.rt0.rgb = color / (1 + color);
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


