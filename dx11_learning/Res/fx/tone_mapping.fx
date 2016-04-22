#ifndef TONE_MAPPING
#define TONE_MAPPING

float3 filmicTonemap(float3 x) {
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((x*(A*x + C*B) + D*E) / (x*(A*x + B) + D*F)) - E / F;
}

float3 ApplyFilmicToneMap(float3 color, float exposure)
{
	color = 2.0f * filmicTonemap(exposure * color);
	float3 whiteScale = 1.0f / filmicTonemap(11.2);
	color *= whiteScale;
	return color;
}




#endif