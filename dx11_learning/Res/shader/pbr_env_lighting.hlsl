cbuffer cbPerFrame : register(b0)
{
	float4x4 g_f4x4World;
	float4x4 g_f4x4View;
	float4x4 g_f4x4Project;
	float3   g_camaraPos;
	float    g_padding;
}

const float		   g_fTextureGamma = 2.0;
SamplerState	   g_SampleLinear          : register(s0);
SamplerState	   g_SampleTrilinear       : register(s1);

Texture2D          g_Albedo                : register(t0);
Texture2D		   g_Normal                : register(t1);
Texture2D		   g_Specular              : register(t2);
TextureCube        g_DiffuseIBL            : register(t3);
TextureCube        g_SpecularIBL           : register(t4);
Texture2D          g_SpecularInterger      : register(t5);

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
	float2 f2TexCoord   : TEXTURE;
	float3 f3WorldPos   : POSITION;
};


// 单片元单输出
struct PS_SingleOutput
{
	float4 rt0    : SV_Target0;
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
	float4 texNormal = g_Normal.SampleLevel(g_SampleLinear, I.f2TexCoord, 0);
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

float4 resovleAlbedo(float4 albedo, float gamma)
{
	return pow(albedo,  gamma);
}


//顶点着色，普通模型
PS_TranslateInput VS_FillBuffer(VS_VertexLayout I)
{
	PS_TranslateInput O;
	float4 posMS = float4(I.f3Position, 1);
	float4 posWS = mul(posMS, g_f4x4World);
	float4 posVS = mul(posWS, g_f4x4View);

	O.f3WorldPos = posWS.rgb;
	//ndc位置
	O.f4Position = mul(posVS, g_f4x4Project);
	//normal 
	O.f3Normal =   normalize( mul(I.f3Normal, (float3x3)g_f4x4World) );
	//coord
	O.f2TexCoord = I.f2TexCoord;

	return O;
}

//片元，普通模型
PS_SingleOutput PS_FillBuffer(PS_TranslateInput I)
{
	PS_SingleOutput O;
	float2 coord = I.f2TexCoord;
	float4 albedo = g_Albedo.SampleLevel(g_SampleLinear, coord, 0);
	//albedo = resovleAlbedo(albedo, 1.8);
	float3 normalWarp = resolveNormal(I);
	float3 RMB = g_Specular.SampleLevel(g_SampleLinear, coord, 0);

	float glossness = 1.0f - RMB.x;
	float matelness = RMB.y;
	float bakedAO = RMB.z;

	float4 diffuseIBL = g_DiffuseIBL.SampleLevel(g_SampleLinear, normalWarp, 0) / 3.14159;
	
	float3 view = normalize(g_camaraPos - I.f3WorldPos);
	float3 refl = normalize(reflect(-view, normalize(normalWarp)));
	float VoN = dot(view, normalWarp);
	float2 brdfTerm = g_SpecularInterger.SampleLevel(g_SampleLinear, float2(VoN, glossness), 0).xy;
	float4 specularIBL = g_SpecularIBL.SampleLevel(g_SampleTrilinear, refl, glossness*9);

	float3 dielectricColor = float3(0.04, 0.04, 0.04);
	float3 specular = lerp(dielectricColor, albedo.rgb, matelness);

	float3 litColor = diffuseIBL.rgb * albedo + (specularIBL*(specular*brdfTerm.x + brdfTerm.y));
	litColor = litColor * bakedAO;

	O.rt0.rgb = litColor;
	return O;
}
