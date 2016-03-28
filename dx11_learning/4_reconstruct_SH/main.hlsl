cbuffer cbPerObject : register(b0)
{
	float4x4 g_f4x4World;
	float4x4 g_f4x4View;
	float4x4 g_f4x4Project;
	float4x4 colorConvent[3];
}

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState                g_SamplePoint          : register(s0);
Texture2D					tex :					 register(t0);

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


void SH_Y2(float3 n, inout float sh[9])
{
	sh[0] = 0.282094791774f;
	sh[1] = -0.488602511903f * n.y;
	sh[2] = 0.488602511903f * n.z;
	sh[3] = -0.488602511903f * n.x;
	sh[4] = 1.09254843059f * n.x * n.y;
	sh[5] = -1.09254843059f * n.y * n.z;
	sh[6] = 0.315391565253f * (3 * n.z * n.z - 1);
	sh[7] = -1.09254843059f * n.x * n.z;
	sh[8] = 0.546274215296f * (n.x * n.x - n.y * n.y);
}


void SHResult(inout Coeffs c)
{
	c.val[0] = float3(3.182130, 3.087181, 3.509917);
	c.val[1] = float3(-3.712257, -3.683649, -4.291695);
	c.val[2] = float3(-0.026943, -0.024437, -0.031063);
	c.val[3] = float3(0.083974, 0.086743, 0.116283);
	c.val[4] = float3(-0.189730, -0.191554, -0.249978);
	c.val[5] = float3(0.095349, 0.088360, 0.102365);
	c.val[6] = float3(0.001610, 0.001116, 0.000689);
	c.val[7] = float3(9.657115, 9.531387, 11.031955);
	c.val[8] = float3(-2.415086, -2.407502, -2.839465);
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
PS_SingleOutput PS_ResultPass(PS_TranslateInput I)
{
	PS_SingleOutput O;
	float4 normal = float4(normalize(I.f3Normal), 1);
	float2 coord = I.f2TexCoord;
	float sh[9];
	Coeffs result;
	SH_Y2(normal.xyz, sh);
	SHResult(result);
	float3 color = convolve(sh, result);
	O.rt0.rgb = color;
	return O;
}

