#ifndef SH_LIGHTING_HEADER__
#define SH_LIGHTING_HEADER__


struct Coeffs
{
	float4 val[9];
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


float3 ShConvolve(float3 normal, Coeffs result)
{
	float3 color = 0;
	float  sh9[9];
	SH_Y2(normal, sh9);

	for (int i = 0; i < 9; ++i)
	{
		float3 val = result.val[i].rgb * sh9[i];
		color += val;
	}
	return color;
}




#endif