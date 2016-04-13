#ifndef FULL_SCREEN_RPOCESS
#define FULL_SCREEN_PROCESS

#include"base_define.hlsli"


//全屏处理公共顶点着色器
PS_FullScreenInput VS_FullScreenProcess(uint VertexID: SV_VertexID)
{
	PS_FullScreenInput O;
	O.f2TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	O.f4Position = float4(O.f2TexCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return O;
}

#endif