
#define THREAD_DIMESION 4

cbuffer CB_Resolution : register(b0)
{
	float2 resolution;
	int face;
	int padding;
}

SamplerState               g_SampleTriLinear      : register(s0);
TextureCube<float4>        srcTexture : register(t0);
RWTexture2D<float4>        outTexture : register(u0);


float3 GetDirection(float2 uv, int face)
{
	float3 direction = 0;
		switch (face)
	{
		case 0:
			direction = float3(uv, 1);
			break;
		case 1:
			direction = float3(uv, -1);
			break;
		case 2:
			uv = uv.yx;
			direction = float3(1, uv);
			break;
		case 3:
			uv = uv.yx;
			direction = float3(-1, uv);
			break;
		case 4:
			uv.y = -uv.y;
			direction = float3(uv.x, 1, uv.y);
			break;
		case 5:
			direction = float3(uv.x, -1, uv.y);
			break;
		default:
			direction = float3(1, 1, 1);
			break;
	}

	direction = normalize(direction);
	return direction;
}

[numthreads(THREAD_DIMESION, THREAD_DIMESION, 1)]
void copy(uint2 id : SV_DispatchThreadID)
{
	float2 coord = (float2)id / resolution;
	coord.y = 1 - coord.y;
	coord = coord * 2 - 1;

	float3 dir =  GetDirection(coord, 5);
	outTexture[id] = srcTexture.SampleLevel(g_SampleTriLinear, dir, 0);
}
