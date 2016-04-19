#ifndef BASE_DEFINE_HEADER__
#define BASE_DEFINE_HEADER__

#define PI     3.14159265358979323f
#define INV_PI 0.31830988618379067f

//���ڴ���PS����
struct PS_FullScreenInput
{
	float4 f4Position   : SV_Position;
	float2 f2TexCoord   : TEXTURE;
};



// ��ͨģ�͵Ķ������
struct VS_Input_Xyznuv
{
	float3 f3Position    : POSITION;
	float3 f3Normal      : NORMAL;
	float2 f2TexCoord    : TEXCOORD0;
};

// ��ƬԪ�����
struct PS_Output_Single
{
	float4 color0    : SV_Target0;
};


Texture2D                   Albedomap :                  register(t0);
Texture2D                   Normalmap :                  register(t1);
Texture2D                   Specularmap :                register(t2);
Texture2D                   Emitmap :					 register(t3);
TextureCube                 Irridiancemap :              register(t4);
TextureCube                 SpecularLukup:               register(t5);
Texture2D                   IntergeLukupmap:             register(t6);
TextureCube                 EnvCubemap:					 register(t7);
Texture2D                   Texture0:				     register(t8);
Texture2D                   Texture1:					 register(t9);
Texture2D                   Texture2:					 register(t10);
Texture2D                   Cubemap1:					 register(t11);
Texture2D                   Cubemap2:					 register(t12);

SamplerState                AlbedoSampler:               register(s0);
SamplerState                NormalSampler:               register(s1);
SamplerState                SpecularSampler:             register(s2);
SamplerState                EmitSampler:				 register(s3);
SamplerState                IrridianceSampler:           register(s4);
SamplerState                SpecularLukupSampler:        register(s5);
SamplerState                IntergeSampler:              register(s6);
SamplerState                EnvCubeSampler:              register(s7);



#endif