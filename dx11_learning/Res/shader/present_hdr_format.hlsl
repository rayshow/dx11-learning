#include"full_screen_process.hlsli"
#include"tone_mapping.hlsli"


PS_Output_Single PS_present_hdr(PS_FullScreenInput I)
{
	PS_Output_Single O;
	O.color0 = float4(0.2, 0, 0, 1);

	return O;
}




