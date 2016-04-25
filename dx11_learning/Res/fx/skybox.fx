#include"base_define.fx"


float4x4 RotateProject: RotateProjectMatrix;
TextureCube EnvCubemap:EnvCubeMap;



//普通模型的PS参数
struct PS_TranslateInput
{
	float4 f4Position   : SV_Position;
	float3 f3Normal     : COLOR1;
	float3 f3Dir		: COLOR2;
	float2 f2TexCoord   : TEXTURE;
};

//顶点着色，普通模型
PS_TranslateInput VS_FillBuffer(VS_Input_Xyznuv I)
{
	PS_TranslateInput O;

	float4 posPS = mul(float4(I.f3Position, 1), RotateProject);

	O.f3Dir = normalize(I.f3Position);

	//ndc位置
	O.f4Position = posPS;

	//normal 
	O.f3Normal = I.f3Normal;
	//coord
	O.f2TexCoord = I.f2TexCoord;

	return O;
}

//片元，普通模型
PS_Output_Single PS_FillBuffer(PS_TranslateInput I)
{
	PS_Output_Single O;
	O.color0 = EnvCubemap.SampleLevel(anisotropicSampler, I.f3Dir, 0);
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
