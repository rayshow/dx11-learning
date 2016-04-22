#ifndef D3D_HEADER__
#define D3D_HEADER__

//#include<d3dcommon.h>
#include<d3d11.h>
#include<string>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include"util/UlHelper.h"
#include"resource/ResourceMgr.h"

using std::string;

namespace ul
{
	static const float     BLACK_CLEAR_COLOR[4] = { 0, 0, 0, 1 };
	static const float     WHITE_CLEAR_COLOR[4] = { 1, 1, 1, 1 };

	class D3D11GraphicsContext
	{
	private:
		bool			         enableVsyn_;
		bool			         enableFullScreen_;
		bool                     enable4xMsaa_;
		ulUint                   msaaQuality_;
		int				         videoMemory_;
		string			         videoCardDescription_;
		IDXGISwapChain*          pSwapChain_;
		ID3D11Device*            pDevice_;
		ID3D11DeviceContext*     pContext_;
		ID3D11RenderTargetView*  pMainRT_;
		ID3D11DepthStencilView*  pMainDSV_;
		ID3D11DepthStencilState* pDepthStenilState_;
		ID3D11RasterizerState*   pRasterState_;
		ResourceMgr*             pResourceMgr_;
		ID3D11Texture2D*         pDepthStencil2D_;
		D3D11_VIEWPORT           viewport_;
		DXGI_SWAP_CHAIN_DESC     swapChainDesc_;
		D3D11_TEXTURE2D_DESC     depthBufferDesc_;
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc_;
		D3D11_DEPTH_STENCIL_VIEW_DESC		dsvDesc_;
		D3D11_RASTERIZER_DESC	 rasterDesc_;

	public:
		D3D11GraphicsContext() :
			enableVsyn_(true),
			enableFullScreen_(false),
			enable4xMsaa_(false),
			msaaQuality_(0),
			videoMemory_(0),
			videoCardDescription_(""),
			pSwapChain_(nullptr),
			pDevice_(nullptr),
			pContext_(nullptr),
			pMainRT_(nullptr),
			pMainDSV_(nullptr),
			pDepthStenilState_(nullptr),
			pResourceMgr_(nullptr),
			pDepthStencil2D_(nullptr)
		{};

		~D3D11GraphicsContext()
		{
		}

	public:
		bool Initialize(HWND hwnd, int width,int height,
			bool vsync, bool fullScreen, bool enable4xMsaa);

		void Shutdown();
		void Resize(ulUint width, ulUint height);

		void EndScene() { pSwapChain_->Present(0, 0); }

		ID3D11Device* GetDevicePtr() { return pDevice_; }

		ID3D11DeviceContext* GetDeviceContextPtr() { return pContext_; }

		ID3D11RenderTargetView* GetMainRenderTargetPtr() { return pMainRT_; }

		ID3D11DepthStencilView* GetMainDepthStencilViewPtr() { return pMainDSV_; }

		void BeginScene()
		{
			pContext_->ClearRenderTargetView(pMainRT_, BLACK_CLEAR_COLOR);
			pContext_->ClearDepthStencilView(pMainDSV_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}


	};
};

#endif