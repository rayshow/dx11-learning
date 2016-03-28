
//
// References
//
//    [1] Sloan, P.-P., Luna, B., and Snyder, J. 2005.
//        Local, deformable precomputedradiance transfer.
//        ACM Trans. Graph. 24, 3, 1216–1223
//
//    [2] Stupid Spherical Harmonics (SH) Tricks
//        Peter-Pike Sloan, Microsoft Corporation
//


#ifndef INCLUDED_SPHERICAL_HARMONICS_H
#define INCLUDED_SPHERICAL_HARMONICS_H


#include <cbpp/Math.h>


#if defined(__cplusplus)
namespace math
{
#endif


	/*$pycgen
	from math import pi
	from math import sqrt

	k_2sqrtpi = 2 * sqrt(pi)
	k_4sqrtpi = 4 * sqrt(pi)

	# Constants from [2], Appendix A2 Polynomial Forms of SH Basis
	y = [
		1.0 / k_2sqrtpi,
		-sqrt(3) / k_2sqrtpi,
		sqrt(3) / k_2sqrtpi,
		-sqrt(3) / k_2sqrtpi,
		sqrt(15) / k_2sqrtpi,
		-sqrt(15) / k_2sqrtpi,
		sqrt(5) / k_4sqrtpi,
		-sqrt(15) / k_2sqrtpi,
		sqrt(15) / k_4sqrtpi
	]
	*/


	// Return single DC-term for order 0 SH
	cmp_device_fn float SH_Y0()
	{
		/*$pycgen
		code = """
		return {y[0]}f;
		"""
		EmitFmt(code)
		*/
		//$pycgen-begin
		return 0.282094791774f;
		//$pycgen-end
	}


	// Calculate order 1 SH co-efficients along the given direction
	cmp_device_fn void SH_Y1(float3 n, cmp_out float sh[4])
	{
		/*$pycgen
		code = """
		sh[0] = {y[0]}f;
		sh[1] = {y[1]}f * n.y;
		sh[2] = {y[2]}f * n.z;
		sh[3] = {y[3]}f * n.x;
		"""
		EmitFmt(code)
		*/
		//$pycgen-begin
		sh[0] = 0.282094791774f;
		sh[1] = -0.488602511903f * n.y;
		sh[2] = 0.488602511903f * n.z;
		sh[3] = -0.488602511903f * n.x;
		//$pycgen-end
	}


	// Calculate order 2 SH co-efficients along the given direction
	cmp_device_fn void SH_Y2(float3 n, cmp_out float sh[9])
	{
		/*$pycgen
		code += """
		sh[4] = {y[4]}f * n.x * n.y;
		sh[5] = {y[5]}f * n.y * n.z;
		sh[6] = {y[6]}f * (3 * n.z * n.z - 1);
		sh[7] = {y[7]}f * n.x * n.z;
		sh[8] = {y[8]}f * (n.x * n.x - n.y * n.y);
		"""
		EmitFmt(code)
		*/
		//$pycgen-begin
		sh[0] = 0.282094791774f;
		sh[1] = -0.488602511903f * n.y;
		sh[2] = 0.488602511903f * n.z;
		sh[3] = -0.488602511903f * n.x;
		
		sh[4] = 1.09254843059f * n.x * n.y;
		sh[5] = -1.09254843059f * n.y * n.z;
		sh[6] = 0.315391565253f * (3 * n.z * n.z - 1);
		sh[7] = -1.09254843059f * n.x * n.z;
		sh[8] = 0.546274215296f * (n.x * n.x - n.y * n.y);
		//$pycgen-end
	}


	// Returns SH co-efficients at lm(0,0), lm(1,0) and lm(2, 0) for cosine lobe along positive z-axis.
	// As the cosine lobe is circularly symmetric about the z-axis, all other SH co-efficients are zero.
	// This allows the cosine lobe to be specified as ZH co-efficients.
	cmp_device_fn float3 ZH_CosineLobe2()
	{
		// Projection of cosine lobe onto SH upper hemisphere
		// Mathematica source from Sébastien Lagarde
		// http://www.gamedev.net/topic/631913-cosine-lobe-in-spherical-harmonics/#entry4985124

		/*
		RealSH[L_, M_, \[Theta]_, \[Phi]_] := \[Sqrt]((2 L + 1) / (4 \[Pi]) (L - Abs[M])! / (L + Abs[M])!) *
			LegendreP[L, Abs[M], Cos[\[Theta]]] *
				If[M < 0, \[Sqrt]2 Cos[M.\[Phi]],
				If[M > 0, \[Sqrt]2 Sin[-M.\[Phi]], 1]]

		DiffuseTransfer[\[Theta]_, \[Phi]_] := Max[0, Cos[\[Theta]]]

		ProjectOntoSH[F_, L_, M_] := \!\(
			\*SubsuperscriptBox[\(\[Integral]\), \(0\), \(\[Pi]/2\)]\(
			\*SubsuperscriptBox[\(\[Integral]\), \(0\), \(2  \[Pi]\)]\((RealSH[L, 
				M, \[Theta], \[Phi]] F[\[Theta], \[Phi]] Sin[\[Theta]])\) \
				\[DifferentialD]\[Phi] \[DifferentialD]\[Theta]\)\)

		Table[FullSimplify[ProjectOntoSH[DiffuseTransfer, L, 0], Exponent > 0], {L, 0, 3}]
		*/

		/*$pycgen
		zh_cos = [
			sqrt(pi) / 2,
			sqrt(pi / 3),
			sqrt(5 * pi) / 8
		]

		code = """
		return float3_make({zh_cos[0]}f, {zh_cos[1]}f, {zh_cos[2]}f);
		"""
		EmitFmt(code)
		*/
		//$pycgen-begin
		return float3_make(0.886226925453f, 1.02332670795f, 0.495415912201f);
		//$pycgen-end
	}


	// Returns SH co-efficients for spherical light source that subtends an angle 'a',
	// along the positive z-axis. Stored as zonal harmonics.
	cmp_device_fn float3 ZH_SphericalCap2(float sin_a, float cos_a)
	{
		const float s = sin_a;
		const float c = cos_a;

		// Derived from [2], Appendix A3 ZH Coefficients for Spherical Light Source
		// Can be derived in Mathematica by swapping DiffuseTransfer above for:
		//
		//    SphericalCap[\[Theta]_, \[Phi]_, a_] := \[Theta] < a
		float3 zh;

		/*$pycgen
		zh_sphc_k = [
			-sqrt(pi),
			0.5 * sqrt(3) * sqrt(pi),
			-0.5 * sqrt(5) * sqrt(pi)
		]

		zh_sphc_t = [
			"(-1 + c)",
			"(s * s)",
			"c * (-1 + c) * (c + 1)"
		]

		code = """
		zh.x = {zh_sphc_k[0]}f * {zh_sphc_t[0]};
		zh.y = {zh_sphc_k[1]}f * {zh_sphc_t[1]};
		zh.z = {zh_sphc_k[2]}f * {zh_sphc_t[2]};
		"""

		EmitFmt(code)
		*/
		//$pycgen-begin
		zh.x = -1.77245385091f * (-1 + c);
		zh.y = 1.53499006192f * (s * s);
		zh.z = -1.9816636488f * c * (-1 + c) * (c + 1);
		//$pycgen-end

		return zh;
	}


	// Given the SH function f and ZH function g, convolution of the two is defined as:
	//
	//    f.g(s) = sum[lm] y_lm(s).k.g_l.f_lm		(3)
	//
	// where k is the ZH convolution co-efficient.
	//
	// This can also be used to convert a ZH function to SH along an arbitrary axis, s:
	//
	//    g*_lm = ylm(s).k.g_l						(6)
	//
	// These equations are referenced from [1]
	cmp_device_fn float ZH_ConvolutionCoefficient(float l)
	{
		/*$pycgen
		k_4pi = 4 * pi

		ZH_Convolve = lambda l: sqrt(k_4pi / (2 * l + 1))

		# Complete equation (6) as much as possible before the direction gets applied
		def ZH_ToSHScale(zh):
			sh = [0] * 9
			sh[0] = y[0] * zh[0] * ZH_Convolve(0)
			sh[1] = y[1] * zh[1] * ZH_Convolve(1)
			sh[2] = y[2] * zh[1] * ZH_Convolve(1)
			sh[3] = y[3] * zh[1] * ZH_Convolve(1)
			sh[4] = y[4] * zh[2] * ZH_Convolve(2)
			sh[5] = y[5] * zh[2] * ZH_Convolve(2)
			sh[6] = y[6] * zh[2] * ZH_Convolve(2)
			sh[7] = y[7] * zh[2] * ZH_Convolve(2)
			sh[8] = y[8] * zh[2] * ZH_Convolve(2)
			return sh

		EmitFmt("return sqrtf({k_4pi}f / (2l + 1));")
		*/
		//$pycgen-begin
		return sqrtf(12.5663706144f / (2l + 1));
		//$pycgen-end
	}


	cmp_device_fn float SH_CosineLobe0()
	{
		/*$pycgen
		sh_cos = ZH_ToSHScale(zh_cos)
		EmitFmt("return {sh_cos[0]}f;")
		*/
		//$pycgen-begin
		return 0.886226925453f;
		//$pycgen-end
	}


	cmp_device_fn void  SH_CosineLobe1(float3 n, cmp_out float sh[4])
	{
		/*$pycgen
		code = """
		sh[0] = {sh_cos[0]}f;
		sh[1] = {sh_cos[1]}f * n.y;
		sh[2] = {sh_cos[2]}f * n.z;
		sh[3] = {sh_cos[3]}f * n.x;
		"""
		EmitFmt(code)
		*/
		//$pycgen-begin
		sh[0] = 0.886226925453f;
		sh[1] = -1.02332670795f * n.y;
		sh[2] = 1.02332670795f * n.z;
		sh[3] = -1.02332670795f * n.x;
		//$pycgen-end
	}


	cmp_device_fn void  SH_CosineLobe2(float3 n, cmp_out float sh[9])
	{
		/*$pycgen
		code += """
		sh[4] = {sh_cos[4]}f * n.x * n.y;
		sh[5] = {sh_cos[5]}f * n.y * n.z;
		sh[6] = {sh_cos[6]}f * (3 * n.z * n.z - 1);
		sh[7] = {sh_cos[7]}f * n.x * n.z;
		sh[8] = {sh_cos[8]}f * (n.x * n.x - n.y * n.y);
		"""
		EmitFmt(code)
		*/
		//$pycgen-begin
		sh[0] = 0.886226925453f;
		sh[1] = -1.02332670795f * n.y;
		sh[2] = 1.02332670795f * n.z;
		sh[3] = -1.02332670795f * n.x;
		
		sh[4] = 0.85808553081f * n.x * n.y;
		sh[5] = -0.85808553081f * n.y * n.z;
		sh[6] = 0.2477079561f * (3 * n.z * n.z - 1);
		sh[7] = -0.85808553081f * n.x * n.z;
		sh[8] = 0.429042765405f * (n.x * n.x - n.y * n.y);
		//$pycgen-end
	}


	cmp_device_fn float SH_SphericalCap0()
	{
		/*$pycgen
		sh_sphc = ZH_ToSHScale(zh_sphc_k)
		EmitFmt("return {sh_sphc[0]}f;")
		*/
		//$pycgen-begin
		return -1.77245385091f;
		//$pycgen-end
	}


	cmp_device_fn void  SH_SphericalCap1(float3 n, float sin_a, float cos_a, cmp_out float sh[4])
	{
		const float s = sin_a;
		const float c = cos_a;

		/*$pycgen
		code = """
		float zh_sphc_t0 = {zh_sphc_t[0]};
		float zh_sphc_t1 = {zh_sphc_t[1]};
		sh[0] = {sh_sphc[0]}f * zh_sphc_t0;
		sh[1] = {sh_sphc[1]}f * n.y * zh_sphc_t1;
		sh[2] = {sh_sphc[2]}f * n.z * zh_sphc_t1;
		sh[3] = {sh_sphc[3]}f * n.x * zh_sphc_t1;
		"""
		EmitFmt(code)
		*/
		//$pycgen-begin
		float zh_sphc_t0 = (-1 + c);
		float zh_sphc_t1 = (s * s);
		sh[0] = -1.77245385091f * zh_sphc_t0;
		sh[1] = -1.53499006192f * n.y * zh_sphc_t1;
		sh[2] = 1.53499006192f * n.z * zh_sphc_t1;
		sh[3] = -1.53499006192f * n.x * zh_sphc_t1;
		//$pycgen-end
	}


	cmp_device_fn void  SH_SphericalCap2(float3 n, float sin_a, float cos_a, cmp_out float sh[9])
	{
		const float s = sin_a;
		const float c = cos_a;

		/*$pycgen
		code += """
		float zh_sphc_t2 = {zh_sphc_t[2]};
		sh[4] = {sh_sphc[4]}f * n.x * n.y * zh_sphc_t2;
		sh[5] = {sh_sphc[5]}f * n.y * n.z * zh_sphc_t2;
		sh[6] = {sh_sphc[6]}f * (3 * n.z * n.z - 1) * zh_sphc_t2;
		sh[7] = {sh_sphc[7]}f * n.x * n.z * zh_sphc_t2;
		sh[8] = {sh_sphc[8]}f * (n.x * n.x - n.y * n.y) * zh_sphc_t2;
		"""
		EmitFmt(code)
		*/
		//$pycgen-begin
		float zh_sphc_t0 = (-1 + c);
		float zh_sphc_t1 = (s * s);
		sh[0] = -1.77245385091f * zh_sphc_t0;
		sh[1] = -1.53499006192f * n.y * zh_sphc_t1;
		sh[2] = 1.53499006192f * n.z * zh_sphc_t1;
		sh[3] = -1.53499006192f * n.x * zh_sphc_t1;
		
		float zh_sphc_t2 = c * (-1 + c) * (c + 1);
		sh[4] = -3.43234212324f * n.x * n.y * zh_sphc_t2;
		sh[5] = 3.43234212324f * n.y * n.z * zh_sphc_t2;
		sh[6] = -0.990831824402f * (3 * n.z * n.z - 1) * zh_sphc_t2;
		sh[7] = 3.43234212324f * n.x * n.z * zh_sphc_t2;
		sh[8] = -1.71617106162f * (n.x * n.x - n.y * n.y) * zh_sphc_t2;
		//$pycgen-end
	}


	cmp_device_fn void SH_Scale2(cmp_out float sh_result[9], float scale)
	{
		sh_result[0] *= scale;
		sh_result[1] *= scale;
		sh_result[2] *= scale;
		sh_result[3] *= scale;
		sh_result[4] *= scale;
		sh_result[5] *= scale;
		sh_result[6] *= scale;
		sh_result[7] *= scale;
		sh_result[8] *= scale;
	}


	cmp_device_fn void SH_AddScaled2(cmp_out float sh_result[9], float sh[9], float scale)
	{
		sh_result[0] += sh[0] * scale;
		sh_result[1] += sh[1] * scale;
		sh_result[2] += sh[2] * scale;
		sh_result[3] += sh[3] * scale;
		sh_result[4] += sh[4] * scale;
		sh_result[5] += sh[5] * scale;
		sh_result[6] += sh[6] * scale;
		sh_result[7] += sh[7] * scale;
		sh_result[8] += sh[8] * scale;
	}


	cmp_device_fn float SH_Convolve1(float a[4], float b[4])
	{
		return
			a[0] * b[0] + 
			a[1] * b[1] + 
			a[2] * b[2] + 
			a[3] * b[3];
	}


	cmp_device_fn float SH_Convolve2(float a[9], float b[9])
	{
		return
			a[0] * b[0] + 
			a[1] * b[1] + 
			a[2] * b[2] + 
			a[3] * b[3] + 
			a[4] * b[4] + 
			a[5] * b[5] + 
			a[6] * b[6] + 
			a[7] * b[7] + 
			a[8] * b[8];
	}


#if defined(__cplusplus)
}
#endif


#endif	// INCLUDED_SPHERICAL_HARMONICS_H