float2 TexelIndexToUV(u32 x, u32 y, u32 width, u32 height)
{
	float2 uv;
	uv.x = x / float(width);
	uv.y = y / float(height);
	return uv;
}


float3 UVToRay(const float2& uv)
{
	// Assume end is z=1 plane

	float3 ray;
	ray.x = uv.x * 2 - 1;
	ray.y = uv.y * 2 - 1;
	ray.z = 1;

	return normalize(ray);
}
