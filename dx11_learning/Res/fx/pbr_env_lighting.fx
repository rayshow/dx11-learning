#include"ibl.fx"
#include"base_define.fx"



cbuffer cbPerFrame : register(b0)
{
	uint     specularType;
	uint     irridianceType;
	uint     outputType;
	uint     padding;
}


static const float TextureGamma = 2.0;


#define eSpecular_Ibl  0
#define eSpecular_Realtime_Imp 1

#define eIrridiance_Ibl 0 
#define eIrridiance_Sh  1

#define eTonemapping_Avg_Lumin  0
#define eTonemapping_Filmic 1

#define eOutput_All 0
#define eOutput_Irridiance 1
#define eOutput_Specular 2
#define eOutput_Ao 3
#define eOutput_Roughness 4
#define eOutput_Matelness 5
#define eOutput_Lukup 6
#define eOutput_Normal 7


// 顶点参数
struct VS_VertexLayout
{
	float3 f3Position    : POSITION;
	float3 f3Normal      : NORMAL;
	float2 f2TexCoord    : TEXCOORD0;
};

// PS参数
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float3 f3Position   : COLOR2;
	float2 f2TexCoord   : TEXTURE;
	float3 f3WorldPos   : POSITION;
};



float3x3 invert_3x3(float3x3 M)
{
	float D = determinant(M);
	float3x3 T = transpose(M);

	return float3x3(
	cross(T[1], T[2]),
	cross(T[2], T[0]),
	cross(T[0], T[1])) / (D + 1e-6);
}


float3
resolveNormal(PS_TranslateInput I)
{
	float3 normal = float3(0, 0, 0);
	float3 tangent = float3(0, 0, 0);
	float3 binormal = float3(0, 0, 0);

	// Tangent basis from screen space derivatves. 
	normal = I.f3Normal;
	float3x3 tbnTransform;
	float4 texNormal = Normalmap.SampleLevel(PointSampler, I.f2TexCoord, 0);
	texNormal.xyz = normalize((2.0f * texNormal.xyz) - 1.0f);

	float3 dp1 = ddx_fine(I.f3WorldPos.xyz);
	float3 dp2 = ddy_fine(I.f3WorldPos.xyz);
	float2 duv1 = ddx_fine(I.f2TexCoord.xy);
	float2 duv2 = ddy_fine(I.f2TexCoord.xy);
	float3x3 M = float3x3(dp1, dp2, normalize(normal));
	float3x3 inverseM = invert_3x3(M);
	float3 T = mul(inverseM, float3(duv1.x, duv2.x, 0));
	float3 B = mul(inverseM, float3(duv1.y, duv2.y, 0));
	float scaleT = 1.0f / (dot(T, T) + 1e-6);
	float scaleB = 1.0f / (dot(B, B) + 1e-6);
	tbnTransform[0] = normalize(T*scaleT);
	tbnTransform[1] = -normalize(B*scaleB);
	tbnTransform[2] = normalize(normal);
	tangent = tbnTransform[0];
	normal = normalize((tbnTransform[0] * texNormal.x) + (tbnTransform[1] * texNormal.y) + (tbnTransform[2]));
	return normal;
}

float3 resovleAlbedo(float3 albedo, float gamma)
{
	return pow(albedo,  gamma);
}



//顶点着色，普通模型
PS_TranslateInput VS_FillBuffer(VS_VertexLayout I)
{
	PS_TranslateInput O;
	float4 posMS = float4(I.f3Position, 1);
	float4 posWS = mul(posMS, World);

	O.f3WorldPos = posWS.xyz;
	//ndc位置
	O.f4Position = mul(posMS, WorldViewProject);
	//normal 
	O.f3Normal =   normalize( mul(I.f3Normal, (float3x3)World) );
	//coord
	O.f2TexCoord = I.f2TexCoord;

	O.f3Position = O.f4Position.xyw;
	return O;
}



//片元，普通模型
PS_Output_Single PS_FillBuffer(PS_TranslateInput I)
{
	PS_Output_Single O;
	//O.color0 = float4(0.3, 0, 0, 1);
	//return O;
	float2 coord = I.f2TexCoord;
	float4 albedo = Albedomap.SampleLevel(LinearSampler, coord, 0);
	albedo.rgb = resovleAlbedo(albedo.rgb, 2.2f);
	float3 normalWarp = resolveNormal(I);
	float3 RMB = Specularmap.SampleLevel(LinearSampler, coord, 0);

	float roughness = RMB.x;
	float glossness = 1.0f - roughness;
	float matelness = RMB.y;
	float bakedAO = RMB.z;

	float3 view = normalize(CamaraWorldPos - I.f3WorldPos);
	float3 refl = normalize(reflect(-view, normalize(normalWarp)));
	float VoN = dot(view, normalWarp);


	//environment
	float2 brdfTerm = IntergeLukupmap.SampleLevel(LinearSampler, float2(VoN, roughness), 0).xy;
	float3 specularIBL = 0;
	float3 irridiance = 0;

	
	irridiance = Irridiancemap.SampleLevel(anisotropicSampler, normalWarp, 0) / PI;
	if (specularType == eSpecular_Realtime_Imp)
	{
		specularIBL = PrefilterSpecularMap(
			roughness,
			refl,
			16,
			SpecularLukup,
			anisotropicSampler);
	}
	else{
		specularIBL = SpecularLukup.SampleLevel(anisotropicSampler, refl, glossness * 9.0f).rgb;
	}

	float3 dielectricColor = float3(0.04, 0.04, 0.04);
	float3 specular = lerp(dielectricColor, albedo.rgb, matelness);
	float3 litColor = irridiance * albedo + (specularIBL*(specular*brdfTerm.x + brdfTerm.y));
	litColor = litColor * bakedAO;


	switch (outputType)
	{
	case eOutput_Normal:
		litColor = normalWarp;
		break;
	case eOutput_Irridiance:
		litColor = irridiance.rgb;
		break;
	case eOutput_Specular:
		litColor = specularIBL.rgb;
		break;
	case eOutput_Ao:
		litColor = bakedAO;
		break;
	case eOutput_Roughness:
		litColor = roughness;
		break;
	case eOutput_Matelness:
		litColor = matelness;
		break;
	case eOutput_Lukup:
		litColor = float3(brdfTerm, 0);
		break;
	}


	O.color0.rgb = litColor;//specularIBL;//
	return O;
}



technique11 Default
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_FillBuffer()));
		SetPixelShader(CompileShader(ps_5_0, PS_FillBuffer()));
	}
};

