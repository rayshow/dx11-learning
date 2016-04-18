#ifndef BRDF_HEADER__
#define BRDF_HEADER__


#include"base_define.hlsli"

//Cook-Torrance microfacet specular BRDF
// f(l, v) = D(h)F(v, h)G(l, v, h)/ (4(n*l)(n*v))




// http://graphicrants.blogspot.com.au/2013/08/specular-brdf-reference.html
// Schlick-Beckmann G.
float SchlickGGX(float NoV, float roughness)
{
	float k = roughness / 2;
	return NoV / (NoV * (1.0f - k) + k);
}


float SmithGGX(float NoV, float roughness)
{
	float r2 = pow(roughness, 2);
	return NoV * 2 / (NoV + sqrt((NoV*NoV) * (1.0f - r2) + r2));
}


float SchlickGeometryForLut(float roughness, float NoL)
{
	return SchlickGGX(NoL, roughness * roughness);
}

float SmithGeometryForLut(float roughness, float NoL)
{
	return SmithGGX(NoL, roughness * roughness);
}


// Visibility term
float SmithVisibilityForLut(float roughness, float NoV)
{
	return SmithGGX(NoV, roughness * roughness);
}

// Visibility term
float SchlickVisibilityForLut(float roughness, float NoV)
{
	return SchlickGGX(NoV, roughness * roughness);
}

// Fresnel Term.
float FresnelForLut(float VoH)
{
	return pow(1.0 - VoH, 5);
}



// Trowbridge-Reitz D(h) for GGX.
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
float SpecularD(float roughness, float NoH)
{
	float NoH2 = NoH * NoH;
	float r2 = roughness * roughness;
	return r2 / pow(NoH2 * (r2 - 1.0) + 1.0, 2.0);
}



#endif