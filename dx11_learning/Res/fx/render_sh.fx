#include"base_define.hlsli"

cbuffer cbPerFrame : register(b0)
{
	float4x4 World;
	float4x4 WorldViewProject;
	float4   coeffs[9];
}

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



//普通模型的PS参数
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float2 f2TexCoord   : TEXTURE;
};


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


void SHResult_Factory(inout Coeffs c)
{
	c.val[0] = float3(3.920526, 3.521996, 3.527317);
	c.val[1] = float3(-3.154146, -2.871190, -3.751800);
	c.val[2] = float3(-1.224890, -1.019450, -0.808956);
	c.val[3] = float3(-0.816781, -0.804980, -0.916924);
	c.val[4] = float3(0.733419, 0.849995, 1.229593);
	c.val[5] = float3(0.489887, 0.541333, 0.576702);
	c.val[6] = float3(0.987436, 0.842928, 0.379635);
	c.val[7] = float3(1.439018, 1.238045, 1.059133);
	c.val[8] = float3(-1.424433, -1.445791, -2.356059);
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


//顶点着色，普通模型
PS_TranslateInput VS_RenderCommonMesh(VS_NormalVertexLayout I)
{
	PS_TranslateInput O;

	//ndc位置
	O.f4Position = mul(float4(I.f3Position, 1), WorldViewProject);
	//normal 
	O.f3Normal = normalize(mul(I.f3Normal, (float3x3)World));
	//coord
	O.f2TexCoord = I.f2TexCoord;

	return O;
}



//结果显示
PS_Output_Single PS_FillBufferPass(PS_TranslateInput I)
{
	PS_Output_Single O;

	
	float3 normal = normalize(I.f3Normal);
	float2 coord = I.f2TexCoord;
	float sh[9];
	Coeffs result;
	SH_Y2(normal, sh);
	SHResult_Factory(result);
	float3 color = convolve(sh, result);
		O.color0.rgb = color;// / (1 + color);
	

	return O;
}




