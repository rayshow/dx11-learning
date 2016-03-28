#ifndef D3D11_HELPER_HEADER__
#define D3D11_HELPER_HEADER__

#include <d3d11.h>
#include <D3DX11tex.h>
#include <D3DX11core.h>
#include <D3DX11async.h>

#include "tools.h"

namespace ul
{

	inline void 
		Texture2D_Desc(
		D3D11_TEXTURE2D_DESC& desc,
		ulUint width, ulUint height,
		ulUint mip, ulUint arraySize,
		DXGI_FORMAT format, ulUint sampleCount, ulUint quality,
		D3D11_USAGE usage, ulUint bindFlag,
		ulUint mixFlag, ulUint cPUAccessFlags)
	{
		memset(&desc, 0, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = mip;
		desc.ArraySize = arraySize;
		desc.Format = format;
		desc.CPUAccessFlags = cPUAccessFlags;
		desc.SampleDesc.Count = sampleCount;
		desc.SampleDesc.Quality = quality;
		desc.Usage = usage;
		desc.BindFlags = bindFlag;
		desc.MiscFlags = mixFlag;
	}

};







#endif