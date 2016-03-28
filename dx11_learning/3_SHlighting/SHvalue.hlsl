
#define THREAD_DIMESION 16

struct Coeffs
{
	float3 val[9];
};

cbuffer CB_Resolution : register(b0)
{
	float4 resolution;
}
Texture2D<float4> srcTexture: register(t0);
RWStructuredBuffer<Coeffs> dataIN : register(u0);
RWStructuredBuffer<Coeffs> dataOut : register(u1);


void updatecoeffs(float3 color, float domega, float x, float y, float z, inout Coeffs c) {
	c.val[0] += color *  0.282095 * domega;
	c.val[1] += color * (1.092548 * y)*domega;
	c.val[2] += color * (1.092548 * z)*domega;
	c.val[3] += color * (1.092548 * x)*domega;
	c.val[4] += color * (1.092548 * x*y)*domega;
	c.val[5] += color * (1.092548 * y*z)*domega;
	c.val[6] += color * (0.315392 * x*z)*domega;
	c.val[7] += color * (1.092548 * (3 * z*z - 1))*domega;
	c.val[8] += color * (0.315392 * (x*x - y*y))*domega;
}

[numthreads(THREAD_DIMESION, THREAD_DIMESION, 1)]
void FirstPass(uint2 id : SV_DispatchThreadID)
{
	Coeffs first;
	//first.val[0] = srcTexture[id].rgb * 1;
	//first.val[1] = srcTexture[id].rgb * 2;
	//first.val[2] = srcTexture[id].rgb * 3;
	//first.val[3] = srcTexture[id].rgb * 4;
	//first.val[4] = srcTexture[id].rgb * 5;
	//first.val[5] = srcTexture[id].rgb * 6;
	//first.val[6] = srcTexture[id].rgb * 7;
	//first.val[7] = srcTexture[id].rgb * 8;
	//first.val[8] = srcTexture[id].rgb * 9;
	first.val[0] =  1;
	first.val[1] =  2;
	first.val[2] =  3;
	first.val[3] =  4;
	first.val[4] =  5;
	first.val[5] =  6;
	first.val[6] =  7;
	first.val[7] =  8;
	first.val[8] =  9;

	dataOut[ id.y*1024+id.x ] = first;
}



[numthreads(THREAD_DIMESION, THREAD_DIMESION, 1)]
void NextPass(uint2 id : SV_DispatchThreadID)
{
	uint destIndex = id.y * resolution.x + id.x;
	uint srcIndex = destIndex * 2;
	Coeffs next;
	for (int i = 0; i < 9; ++i)
	{
		next.val[i] = dataIN[srcIndex].val[i] + dataIN[srcIndex + 1].val[i] + dataIN[srcIndex + resolution.x].val[i] + dataIN[srcIndex + resolution.x + 1].val[i];
	}


	dataOut[destIndex] = next;
}
