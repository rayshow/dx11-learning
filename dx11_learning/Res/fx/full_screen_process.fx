#ifndef FULL_SCREEN_RPOCESS
#define FULL_SCREEN_PROCESS

#include"base_define.fx"
#include"tone_mapping.fx"

cbuffer cbParameter : register(b0)
{
	float4 mipLevel;
}

cbuffer HDR_Parameter : register(b0)
{
	float  Exposure;
	float3 padding;
}

SamplerState     pointSampler         	 	  : register(s0);
Texture2D        needProcessTex               : register(t0);


//全屏处理公共顶点着色器
PS_FullScreenInput VS_FullScreenProcess(uint VertexID: SV_VertexID)
{
	PS_FullScreenInput O;
	O.f2TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	O.f4Position = float4(O.f2TexCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return O;
}

//显示结果
PS_Output_Single PS_present_screen(PS_FullScreenInput I)
{
	PS_Output_Single O;
	float4 color = needProcessTex.SampleLevel(pointSampler, I.f2TexCoord, 0);

	O.color0 = color;
	return O;
}


//处理hdr
PS_Output_Single PS_present_hdr(PS_FullScreenInput I)
{
	PS_Output_Single O;
	float4 inColor = needProcessTex.SampleLevel(pointSampler, I.f2TexCoord, 0);
	//float3 color = ApplyFilmicToneMap(inColor.rgb, Exposure);
	//color = pow(color, 1.0f / 2.2f);
	O.color0.rgb = inColor.rgb;
	return O;
}



#endif