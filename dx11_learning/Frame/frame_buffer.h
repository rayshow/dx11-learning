#ifndef UL_FRAME_BUFFER_HEADER__
#define UL_FRAME_BUFFER_HEADER__

#include<d3d11.h>
#include"util\tools.h"

namespace ul
{
	class Dx11Framebuffer
	{
	protected:
		ulUint width_;
		ulUint height_;
	public:
		Dx11Framebuffer(ulUint width, ulUint height, DXGI_FORMAT format)

	};
};








#endif