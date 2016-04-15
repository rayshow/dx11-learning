#include"PostProcess.h"
#include"../resource/ResourceMgr.h"

using namespace ul;

bool PostProcess::Create(
	const string& fileName,
	const string& psFunction,
	DXGI_FORMAT format,
	ulUint width, ulUint height,
	ID3D11Texture2D *owner = nullptr,
	ulUint mipLevel = 0)
{
	ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();
	width_ = width;
	height_ = height;
	format_ = format;	

	//from a exists texture2d
	if (Null(owner))
	{
		//entirely single texture2d
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 0;
		texDesc.ArraySize = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		Null_Return_False_With_Msg(
			(owner = mgr->CreateTexture2DNoData(texDesc, ResourceMgr::eRecyclePerSwapChain)),
			"create single mipmaps texture from postprocess shader file %s : %s error",
			fileName.c_str(),
			psFunction.c_str()
		);
	}

	srvOwnerTexture_ = owner;


	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Null_Return_False((renderTarget_ = mgr->CreateRenderTargetView(owner,
		&rtvDesc, ResourceMgr::eRecyclePerSwapChain)));


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = format;
	Null_Return_False_With_Msg(
		(nextStepProcessSRV_ = mgr->CreateShaderResourceView(owner, &srvDesc, ResourceMgr::eRecyclePerSwapChain)),
		"create postprocess rendertarget from shader %s : %s error",
		fileName.c_str(),
		psFunction.c_str()
	);

	Null_Return_False_With_Msg(
		(parameterBuffer_ = mgr->CreateConstantBuffer(sizeof(SPostProcess_Parameter))),
		"create postprocess const buffer error."
	);

	Null_Return_False((fullScreenVs_ = mgr->CreateVertexShaderFromResourceBasePath("shader/full_screen_process.hlsli", 
						"VS_FullScreenProcess", "vs_5_0")));
	Null_Return_False((fullScreenPs_ = mgr->CreatePixelShaderFromResourceBasePath(fileName.c_str(), 
						psFunction.c_str(), "ps_5_0")));

	SPostProcess_Parameter parameter;
	memset(&parameter, 0, sizeof(parameter));
	parameter.mipLevel = mipLevel;
	mgr->MappingBufferWriteOnly(parameterBuffer_, &parameter, sizeof(parameter));


	return true;
}


bool PostPresent::Create()
{
	ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();
	Null_Return_False((fullScreenVs_ = mgr->CreateVertexShaderFromResourceBasePath("shader/full_screen_process.hlsli",
		"VS_FullScreenProcess", "vs_5_0")));
	Null_Return_False((fullScreenPs_ = mgr->CreatePixelShaderFromResourceBasePath("shader/full_screen_process.hlsli",
		"PS_present_screen", "ps_5_0")));
	return true;
}

bool HdrPresentProcess::Create(
	ulUint width, ulUint height)
{
	//output to 8888 format
	return PostProcess::Create("shader/full_screen_process.hlsli",
		"PS_present_hdr", DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
}

bool PostProcessChain::Create(ulUint width, ulUint height)
{
	this->RemoveAllProcess();

	ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	Null_Return_False((srcTexture_ = mgr->CreateTexture2DNoData(texDesc, ResourceMgr::eRecyclePerSwapChain)));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = texDesc.Format;
	Null_Return_False((srcResourceView_ = mgr->CreateShaderResourceView(srcTexture_, 
		&srvDesc, ResourceMgr::eRecyclePerSwapChain)));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Null_Return_False((srcRenderTarget_ = mgr->CreateRenderTargetView(srcTexture_,
		&rtvDesc, ResourceMgr::eRecyclePerSwapChain)));

	width_ = width;
	height_ = height;

	//present
	False_Return_False(present_.Create());
	present_.SetNeedPresentTexture(srcResourceView_);

	return true;
}