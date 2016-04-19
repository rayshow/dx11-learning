#include<memory>
#include"D3D11GraphicContext.h"

using namespace ul;

bool D3D11GraphicsContext::Initialize(
	HWND hwnd, int width, 
	int height, bool vsync, 
	bool fullScreen)
{
	ulUint numberModes, numerator, denominator;
	AutoReleasePtr<IDXGIFactory> factoryPtr;
	AutoReleasePtr<IDXGIAdapter> adaptorPtr;
	AutoReleasePtr<ID3D11Texture2D> backBufferTexturePtr;
	AutoReleasePtr<ID3D11Texture2D> depthStencilTexturePtr;
	AutoReleasePtr<IDXGIOutput> adaptorOutputPtr;

	DXGI_ADAPTER_DESC adapterDesc;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	D3D11_RASTERIZER_DESC  rasterDesc;
	
	Log_Info("D3D device and context initializing.");

	enableVsyn_ = vsync;
	enableFullScreen_ = fullScreen;
	Fail_Return_False( CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)factoryPtr.GetPtr()));
	Fail_Return_False(factoryPtr->EnumAdapters(0, adaptorPtr.GetPtr() ));
	Fail_Return_False(adaptorPtr->EnumOutputs(0,  adaptorOutputPtr.GetPtr() ));
	Fail_Return_False(adaptorOutputPtr->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED, &numberModes, nullptr));
	
	//avoid excpetion cause mem-leak
	std::unique_ptr<DXGI_MODE_DESC[]> displayModeListPtr(Ul_New DXGI_MODE_DESC[numberModes]);
	Fail_Return_False(displayModeListPtr);

	Fail_Return_False(adaptorOutputPtr->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED, &numberModes, displayModeListPtr.get()));

	for (ulUint i = 0; i < numberModes; ++i)
	{
		//Log_Info("width %d height %d", displayModeListPtr[i].Width, displayModeListPtr[i].Height);
		if (displayModeListPtr[i].Width == width &&
			displayModeListPtr[i].Height == height)
		{
			numerator = displayModeListPtr[i].RefreshRate.Numerator;
			denominator = displayModeListPtr[i].RefreshRate.Denominator;
		}
	}

	Fail_Return_False(adaptorPtr->GetDesc(&adapterDesc));
	videoMemory_ = adapterDesc.DedicatedVideoMemory / (1024 * 1024);
	videoCardDescription_ = WStringToString(adapterDesc.Description);

	//construct swap chain desc
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	numerator = 0;
	denominator = 1;
	if (enableVsyn_)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	if (enableFullScreen_)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	featureLevel = D3D_FEATURE_LEVEL_11_0;
	
	//device and device context
	Fail_Return_False(  D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
		nullptr, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc,
		&swapChain_, &device_, nullptr, &deviceContext_) );
	swapChainDesc_ = swapChainDesc;


	//get main RT
	Fail_Return_False(swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(void**)backBufferTexturePtr.GetPtr() ));
	Fail_Return_False(device_->CreateRenderTargetView(backBufferTexturePtr.Get(),
		nullptr, &mainRT_));

	//main depth stencil rt
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	Fail_Return_False(device_->CreateTexture2D(&depthBufferDesc,
		NULL, depthStencilTexturePtr.GetPtr() ));
	
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	Fail_Return_False(device_->CreateDepthStencilView(depthStencilTexturePtr.Get(), &dsvDesc, &mainDSV_));

	// Setup the depth stencil pass state
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	Fail_Return_False(device_->CreateDepthStencilState(&depthStencilDesc, &depthStenilState_));

	// Setup the raster state
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	Fail_Return_False(device_->CreateRasterizerState(&rasterDesc, &rasterState_));
	
	//set state
	viewport_.Width = (float)width;
	viewport_.Height = (float)height;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	deviceContext_->RSSetViewports(1, &viewport_);
	deviceContext_->RSSetState(rasterState_);
	deviceContext_->OMSetRenderTargets(1, &mainRT_, mainDSV_);
	deviceContext_->OMSetDepthStencilState(depthStenilState_, 1);

	Log_Info("D3D device and context initialized.");

	resourceMgr_ = new ResourceMgr();
	resourceMgr_->init(device_, deviceContext_);
	return true;
}

void D3D11GraphicsContext::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (swapChain_)
	{
		swapChain_->SetFullscreenState(false, nullptr);
		
	}
	resourceMgr_->ReleaseLoadedResourcePerSwapChain();
	resourceMgr_->ReleaseLoadedResourceOnExit();
	Safe_Delete(resourceMgr_);

	Safe_Release(rasterState_);
	Safe_Release(mainDSV_);
	Safe_Release(mainRT_);
	Safe_Release(depthStenilState_);
	Safe_Release(deviceContext_);
	Safe_Release(device_);
	Safe_Release(swapChain_);
}