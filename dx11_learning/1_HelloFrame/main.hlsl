


texture2D  tex  : register(t0);
SamplerState  PointSampler          : register(s0);


//普通模型的PS参数
struct PS_FullScreenInput
{
	float4 f4Position   : SV_Position;
	float2 f2TexCoord   : TEXTURE;
};

// 单片元单输出
struct PS_SingleOutput
{
	float4 rt0    : SV_Target0;
};

//全屏处理公共顶点着色器
PS_FullScreenInput VS_FullScreenProcess(uint VertexID: SV_VertexID)
{
	PS_FullScreenInput O;
	O.f2TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	O.f4Position = float4(O.f2TexCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return O;
}


//垂直模糊
PS_SingleOutput PS_FullScreenProcess(PS_FullScreenInput I)
{
	PS_SingleOutput O;
	float2 coord = I.f2TexCoord;
	O.rt0 = tex.SampleLevel(PointSampler, coord, 0);
	return O;
}

