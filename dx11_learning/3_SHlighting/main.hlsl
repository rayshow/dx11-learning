
#define THREAD_DIMESION 4


struct Coeffs
{
	float3 val[9];
	float  sumWeight;

};

cbuffer CB_Resolution : register(b0)
{
	float2 resolution;
	int face;
	int padding;
}

SamplerState               TriLinearSampler      : register(s0);
TextureCube<float4>        srcTexture			 : register(t0);
RWStructuredBuffer<Coeffs> dataIN			     : register(u0);
RWStructuredBuffer<Coeffs> dataOut				 : register(u1);

void updatecoeffs2( inout Coeffs c) {
	c.val[0] = 1;
	c.val[1] = 2;
	c.val[2] = 3;
	c.val[3] = 4;
	c.val[4] = 5;
	c.val[5] = 6;
	c.val[6] = 7;
	c.val[7] = 8;
	c.val[8] = 9;
	c.sumWeight = 1;
}

void updatecoeffs(float3 color, float solidAngleWeight, float3 dir, inout Coeffs c) {
	c.val[0] = color *  0.282094791774f * solidAngleWeight;
	c.val[1] = color * (-0.488602511903f * dir.y)*solidAngleWeight;
	c.val[2] = color * (0.488602511903f * dir.z)*solidAngleWeight;
	c.val[3] = color * (-0.488602511903f * dir.x)*solidAngleWeight;
	c.val[4] = color * (1.09254843059f * dir.x*dir.y)*solidAngleWeight;
	c.val[5] = color * (-1.09254843059f * dir.y*dir.z)*solidAngleWeight;
	c.val[6] = color * (0.315391565253f * dir.x*dir.z)*solidAngleWeight;
	c.val[7] = color * (-1.09254843059f * (3 * dir.z*dir.z - 1))*solidAngleWeight;
	c.val[8] = color * (0.546274215296f * (dir.x*dir.x - dir.y*dir.y))*solidAngleWeight;
	c.sumWeight = solidAngleWeight;
}

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


float AreaElement(float2 xy)
{
	float x = xy.x;
	float y = xy.y;
	return atan2(x * y, sqrt(x * x + y * y + 1));
}


float TexelCoordSolidAngle(float2 uv, float  invResolution)
{
	float2 uv0 = uv;
	float2 uv1 = float2(uv.x, uv.y - invResolution);
	float2 uv2 = float2(uv.x - invResolution, uv.y);
	float2 uv3 = float2(uv.x - invResolution, uv.y - invResolution);
	return AreaElement(uv0) + AreaElement(uv3) - AreaElement(uv1) - AreaElement(uv2);
	return 0;
}

[numthreads(THREAD_DIMESION, THREAD_DIMESION, 1)]
void FirstPass(uint2 id : SV_DispatchThreadID)
{
	Coeffs first;

	float2 uv = (float2)id / resolution;
	uv.y = 1 - uv.y;
	uv = uv * 2 - 1;
	float3 dir = GetDirection(uv, face);

	float3 color = srcTexture.SampleLevel(TriLinearSampler, dir, 0);

	float tmp = 1 + uv.x*uv.x + uv.y*uv.y;
	float solidAngleWeight =  4 / (tmp*sqrt(tmp));
	//float solidAngleWeight = TexelCoordSolidAngle(uv, 1.0f / resolution.x);

	updatecoeffs(color, solidAngleWeight, dir, first);
	//updatecoeffs2(first);

	dataOut[id.y * resolution.x + id.x] = first;
}

[numthreads(THREAD_DIMESION, THREAD_DIMESION, 1)]
void NextPass(uint2 id : SV_DispatchThreadID)
{
	uint destIndex = id.y * resolution.x + id.x;
	id = id * 2;

	uint srcResolution = 2 * resolution.x;
	uint srcIndex1 = id.y * srcResolution + id.x;
	uint srcIndex2 = srcIndex1 + 1;
	uint srcIndex3 = srcIndex1 + srcResolution;
	uint srcIndex4 = srcIndex1 + srcResolution + 1;

	Coeffs next;
	for (int i = 0; i < 9; ++i)
	{
		next.val[i] = dataIN[srcIndex1].val[i] + dataIN[srcIndex2].val[i] + dataIN[srcIndex3].val[i] + dataIN[srcIndex4].val[i];
	}
	next.sumWeight = dataIN[srcIndex1].sumWeight + dataIN[srcIndex2].sumWeight + dataIN[srcIndex3].sumWeight + dataIN[srcIndex4].sumWeight;

	dataOut[destIndex] = next;
}
