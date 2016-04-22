#include<memory>
#include"D3D11GraphicContext.h"
#include<DxErr.h>

using namespace ul;

bool D3D11GraphicsContext::Initialize(
	HWND hwnd, int width, 
	int height, bool vsync = true, 
	bool fullScreen = false, bool enable4xMsaa = false)
{
	Log_Info("D3D device and context initializing.");

	ulUint numberModes, numerator, denominator;
	ulUint deviceFlag = 0;
	AutoReleasePtr<IDXGIFactory> factoryPtr;
	AutoReleasePtr<IDXGIAdapter> adaptorPtr;
	AutoReleasePtr<ID3D11Texture2D> backBufferTexturePtr;
	AutoReleasePtr<ID3D11Texture2D> depthStencilTexturePtr;
	AutoReleasePtr<IDXGIOutput> adaptorOutputPtr;

	DXGI_ADAPTER_DESC	 adapterDesc;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL	 featureLevel;
	
	
#if defined(DEBUG)|| defined(_DEBUG)
	deviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	enableVsyn_ = vsync;
	enableFullScreen_ = fullScreen;
	enable4xMsaa_ = enable4xMsaa;
	numerator = 0;
	denominator = 1;
	featureLevel = D3D_FEATURE_LEVEL_11_0;
	//create device and deviceContext
	Fail_Return_False(D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlag, 0, 0,
		D3D11_SDK_VERSION, &pDevice_, &featureLevel, &pContext_));

	if (enable4xMsaa)
	{
		Fail_Return_False(pDevice_->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &msaaQuality_));
		False_Return_False((msaaQuality_ > 0));
	}

	Fail_Return_False(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)factoryPtr.GetPtr()));
	Fail_Return_False(factoryPtr->EnumAdapters(0, adaptorPtr.GetPtr() ));
	Fail_Return_False(adaptorPtr->GetDesc(&adapterDesc));
	videoMemory_ = adapterDesc.DedicatedVideoMemory / (1024 * 1024);
	videoCardDescription_ = WStringToString(adapterDesc.Description);

	//construct swap chain desc
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SampleDesc.Count = enable4xMsaa ? 4:1;
	swapChainDesc.SampleDesc.Quality = enable4xMsaa ? msaaQuality_-1: 0;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = !enableFullScreen_;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	Fail_Return_False(factoryPtr->CreateSwapChain(pDevice_, &swapChainDesc, &pSwapChain_));
	
	//resource manager
	Null_Return_False_With_Msg((pResourceMgr_ = new ResourceMgr()),
		"new resource manager failed, memory out.");
	pResourceMgr_->init(pDevice_, pContext_);

	//get main RT
	Fail_Return_False(pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(void**)backBufferTexturePtr.GetPtr() ));
	Null_Return_False( (pMainRT_ = pResourceMgr_->CreateRenderTargetView(
		backBufferTexturePtr.Get(), nullptr, eRelease_OnResize)));

	//main depth stencil rt
	ZeroMemory(&depthBufferDesc_, sizeof(depthBufferDesc_));
	depthBufferDesc_.Width = width;
	depthBufferDesc_.Height = height;
	depthBufferDesc_.MipLevels = 1;
	depthBufferDesc_.ArraySize = 1;
	depthBufferDesc_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc_.SampleDesc.Count = enable4xMsaa?4:1;
	depthBufferDesc_.SampleDesc.Quality = enable4xMsaa ? msaaQuality_-1:0;
	depthBufferDesc_.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc_.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc_.CPUAccessFlags = 0;
	depthBufferDesc_.MiscFlags = 0;
	Null_Return_False(( pDepthStencil2D_ = pResourceMgr_->CreateTexture2DNoData(depthBufferDesc_, eRelease_OnResize) ));
	
	//main depth stencil view
	ZeroMemory(&dsvDesc_, sizeof(dsvDesc_));
	dsvDesc_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc_.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc_.Texture2D.MipSlice = 0;
	Null_Return_False((pMainDSV_ = pResourceMgr_->CreateDepthStencilView(pDepthStencil2D_, &dsvDesc_, eRelease_OnResize)));


	// Setup the depth stencil pass state
	ZeroMemory(&depthStencilDesc_, sizeof(depthStencilDesc_));
	depthStencilDesc_.DepthEnable = true;
	depthStencilDesc_.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc_.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc_.StencilEnable = true;
	depthStencilDesc_.StencilReadMask = 0xFF;
	depthStencilDesc_.StencilWriteMask = 0xFF;
	depthStencilDesc_.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc_.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc_.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc_.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc_.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc_.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc_.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc_.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	Null_Return_False((pDepthStenilState_ = pResourceMgr_->CreateDepthStencilState(depthStencilDesc_)));
	
	// Setup the raster state
	ZeroMemory(&rasterDesc_, sizeof(rasterDesc_));
	rasterDesc_.AntialiasedLineEnable = false;
	rasterDesc_.CullMode = D3D11_CULL_NONE;
	rasterDesc_.DepthBias = 0;
	rasterDesc_.DepthBiasClamp = 0.0f;
	rasterDesc_.DepthClipEnable = true;
	rasterDesc_.FillMode = D3D11_FILL_SOLID;
	rasterDesc_.FrontCounterClockwise = false;
	rasterDesc_.MultisampleEnable = false;
	rasterDesc_.ScissorEnable = false;
	rasterDesc_.SlopeScaledDepthBias = 0.0f;
	Null_Return_False((pRasterState_ = pResourceMgr_->CreateRasterState(rasterDesc_)));
	
	//set state
	ZeroMemory(&viewport_, sizeof(viewport_));
	viewport_.Width = (float)width;
	viewport_.Height = (float)height;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;


	pContext_->RSSetViewports(1, &viewport_);
	pContext_->RSSetState(pRasterState_);
	pContext_->OMSetRenderTargets(1, &pMainRT_, pMainDSV_);
	pContext_->OMSetDepthStencilState(pDepthStenilState_, 1);
	Log_Info("D3D device and context initialized.");
	return true;
}


void D3D11GraphicsContext::Resize(ulUint width, ulUint height)
{
	pResourceMgr_->ReleaseLoadedResourceOnResize();
	Fail_Return_Void(pSwapChain_->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	//main rt
	AutoReleasePtr<ID3D11Texture2D> backBuffer;
	Fail_Return_Void( pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D),(void**)backBuffer.GetPtr()) );
	Null_Return_Void( (pMainRT_ = pResourceMgr_->CreateRenderTargetView(backBuffer.Get(), nullptr, eRelease_OnResize) ));

	//main dsv
	depthBufferDesc_.Width = width;
	depthBufferDesc_.Height = height;
	Null_Return_Void((pDepthStencil2D_ = pResourceMgr_->CreateTexture2DNoData(depthBufferDesc_, eRelease_OnResize)));
	Null_Return_Void((pMainDSV_ = pResourceMgr_->CreateDepthStencilView(pDepthStencil2D_, &dsvDesc_, eRelease_OnResize)));

	//main view port
	viewport_.Width = (float)width;
	viewport_.Height = (float)height;
	pContext_->RSSetViewports(1, &viewport_);
	pContext_->OMSetRenderTargets(1, &pMainRT_, pMainDSV_);
}


void D3D11GraphicsContext::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (pSwapChain_)
	{
		pSwapChain_->SetFullscreenState(false, nullptr);
	}
	
	pResourceMgr_->ReleaseLoadedResourceOnResize();
	pResourceMgr_->ReleaseLoadedResourceOnExit();
	pContext_->ClearState();

	Safe_Delete(pResourceMgr_);
	Safe_Release(pSwapChain_);

	Safe_Release(pContext_);
	Safe_Release(pDevice_);


	int a = 0;
}