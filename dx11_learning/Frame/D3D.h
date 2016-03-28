#ifndef D3D_HEADER__
#define D3D_HEADER__

//#include<d3dcommon.h>
#include<d3d11.h>
#include<string>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include"util/tools.h"
#include"util/res_mgr.h"

using std::string;

namespace ul
{
	static const float     BLACK_CLEAR_COLOR[4] = { 0, 0, 0, 1 };
	static const float     WHITE_CLEAR_COLOR[4] = { 1, 1, 1, 1 };

	class D3D
	{
	public:
	
	private:
		bool            enableVsyn_;
		bool            enableFullScreen_;
		int             videoMemory_;
		string		    videoCardDescription_;
		IDXGISwapChain *swapChain_;
		ID3D11Device   *device_;
		ID3D11DeviceContext *deviceContext_;
		ID3D11RenderTargetView *mainRT_;
		ID3D11DepthStencilView  *mainDSV_;
		ID3D11DepthStencilState *depthStenilState_;
		ID3D11RasterizerState   *rasterState_;
		ResourceMgr    *resourceMgr_;
		D3D11_VIEWPORT  viewport_;
		DXGI_SWAP_CHAIN_DESC  swapChainDesc_;
	public:
		D3D() :
			enableVsyn_(true),
			enableFullScreen_(false),
			videoMemory_(0),
			videoCardDescription_(""),
			swapChain_(nullptr),
			device_(nullptr),
			deviceContext_(nullptr),
			mainRT_(nullptr),
			mainDSV_(nullptr),
			depthStenilState_(nullptr),
			resourceMgr_(nullptr)
		{};

		~D3D()
		{
			
		}

	public:
		bool Initialize(HWND, int, int, bool, bool);
		void Shutdown();
		void BeginScene()
		{
			deviceContext_->ClearRenderTargetView(mainRT_, BLACK_CLEAR_COLOR);
			deviceContext_->ClearDepthStencilView(mainDSV_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,1.0f, 0);
		}

		void Resize(ulUint width, ulUint height)
		{
			
		}
		void EndScene()
		{
			if (enableVsyn_)
				swapChain_->Present(1, 0);
			else
				swapChain_->Present(0, 0);
		}

		ID3D11Device* GetDevicePtr()
		{
			return device_;
		}
		ID3D11DeviceContext* GetDeviceContextPtr()
		{
			return deviceContext_;
		}
		ID3D11RenderTargetView* GetMainRenderTargetPtr()
		{
			return mainRT_;
		}
		ID3D11DepthStencilView* GetMainDepthStencilViewPtr()
		{
			return mainDSV_;
		}
	};
};

#endif