#include<Windows.h>
#include<memory>

#include"util/tools.h"
#include"Application.h"
#include"res_mgr.h"
#include<model_render.h>
#include<D3DX10Math.h>

using namespace ul;


struct CB_PerFrame
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 project;
	XMFLOAT4X4 colorConvent[3];
};

struct CB_EnvMap
{
	XMFLOAT4X4 invProjView;
	XMFLOAT4   eyePos;
};

class Lession1_Frame :public Application
{
public:
	virtual ~Lession1_Frame(){}
public:

	virtual void InitResource(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		ResourceMgr *mgr = ResourceMgr::GetSingletonPtr();

		camara_.LookAt(XMFLOAT4(0, 0, 0, 0), XMFLOAT4(0, 0, 1, 0));

		Null_Return_Void((ball_ = mgr->CreateModelFromFile("../res/mesh/sphere.x")));
		Null_Return_Void((envTexture_ = mgr->CreateTextureFromFile("../res/stpeters_cross.dds")));
		Null_Return_Void((
			commonInputVertexPass_ = mgr->CreateVertexShaderAndInputLayout(
			"main.hlsl", "VS_RenderCommonMesh", "vs_5_0",
			VertexXyznuv_Layout, ARRAYSIZE(VertexXyznuv_Layout), &commonInputLayout_)
		));
		Null_Return_Void((commonInputPixelPass_ = mgr->CreatePixelShader("main.hlsl", "PS_FillBufferPass", "ps_5_0")));
		Null_Return_Void((fullScreenVertex_ = mgr->CreateVertexShader("main.hlsl", "VS_FullScreenProcess", "vs_5_0")));
		Null_Return_Void((fullScreenPixel_ = mgr->CreatePixelShader("main.hlsl", "PS_Display", "ps_5_0")));

		Null_Return_Void((perframeBuffer_ = mgr->CreateConstantBuffer(sizeof(CB_PerFrame))));
		Null_Return_Void((envmapBuffer_ = mgr->CreateConstantBuffer(sizeof(CB_EnvMap))));

		D3D11_SAMPLER_DESC SamDesc;
		SamDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.MipLODBias = 0.0f;
		SamDesc.MaxAnisotropy = 1;
		SamDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0.0;
		SamDesc.MinLOD = 0;
		SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
		SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		Null_Return_Void((TriLinerSampler_ = mgr->CreateSamplerState(SamDesc)));
		SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		Null_Return_Void((PointSampler_ = mgr->CreateSamplerState(SamDesc)));

		D3D11_RASTERIZER_DESC rester;
		memset(&rester, 0, sizeof(rester));
		rester.CullMode = D3D11_CULL_NONE;
		rester.AntialiasedLineEnable = true;
		rester.DepthBias = 0;
		rester.DepthBiasClamp = 0;
		rester.DepthClipEnable = true;
		rester.SlopeScaledDepthBias = 0;
		rester.FillMode = D3D11_FILL_SOLID;
		rester.FrontCounterClockwise = false;
		rester.MultisampleEnable = false;
		rester.ScissorEnable = false;
		Null_Return_Void((resterState_ = mgr->CreateRasterState(rester)));
	};


	virtual void WindowResize(int width, int height,
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		ResourceMgr *mgr = ResourceMgr::GetSingletonPtr();
		mgr->ReleaseLoadedResourcePerSwapChain();

		aspect = (float)width / (float)height;
		XMStoreFloat4x4(&world_, XMMatrixIdentity());
		camara_.SetProject(BaseCamara::eCamara_Perspective, XM_PI / 4, aspect, 0.1f, 1000.0f);

		//one texture
		D3D11_TEXTURE2D_DESC rgba_nomips_Desc;
		rgba_nomips_Desc.Width = UINT(width);
		rgba_nomips_Desc.Height = UINT(height);
		rgba_nomips_Desc.MipLevels = 0;
		rgba_nomips_Desc.ArraySize = 1;
		rgba_nomips_Desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		rgba_nomips_Desc.SampleDesc.Count = 1;
		rgba_nomips_Desc.SampleDesc.Quality = 0;
		rgba_nomips_Desc.Usage = D3D11_USAGE_DEFAULT;
		rgba_nomips_Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		rgba_nomips_Desc.CPUAccessFlags = 0;
		rgba_nomips_Desc.MiscFlags = 0;
		AutoReleasePtr<ID3D11Texture2D> colorTex;
		AutoReleasePtr<ID3D11Texture2D> depthTex;
		Fail_Return_Void(dev->CreateTexture2D(&rgba_nomips_Desc, nullptr, colorTex.GetPtr()));
		Fail_Return_Void(dev->CreateTexture2D(&rgba_nomips_Desc, nullptr, depthTex.GetPtr()));
		
		D3D11_SHADER_RESOURCE_VIEW_DESC allMipmap;
		allMipmap.Format = rgba_nomips_Desc.Format;
		allMipmap.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		allMipmap.Texture2D.MipLevels = 0;
		allMipmap.Texture2D.MostDetailedMip = 0;
		Fail_Return_Void((colorSRV_ = mgr->CreateShaderResourceView(colorTex.Get(), nullptr, ResourceMgr::eRecyclePerSwapChain)));
		Fail_Return_Void((depthSRV_ = mgr->CreateShaderResourceView(depthTex.Get(), nullptr, ResourceMgr::eRecyclePerSwapChain)));

		D3D11_RENDER_TARGET_VIEW_DESC RTDesc;
		RTDesc.Format = rgba_nomips_Desc.Format;
		RTDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTDesc.Texture2D.MipSlice = 0;
		Fail_Return_Void((colorRT_ = mgr->CreateRenderTargetView(colorTex.Get(), nullptr, ResourceMgr::eRecyclePerSwapChain)));
		Fail_Return_Void((depthRT_ = mgr->CreateRenderTargetView(depthTex.Get(), nullptr, ResourceMgr::eRecyclePerSwapChain)));
	};

	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		context->IASetInputLayout(commonInputLayout_);
		context->RSSetState(resterState_);
		context->PSSetSamplers(0, 1, &TriLinerSampler_);

		XMFLOAT4X4 v = camara_.GetViewMatrix();
		XMFLOAT4X4 p = camara_.GetProjectMatrix();
		XMMATRIX invProjView = XMMatrixMultiply(XMLoadFloat4x4(&p), XMLoadFloat4x4(&v));
		XMVECTOR det;
		invProjView = XMMatrixInverse(&det, invProjView);

		//mvp
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		memset(&MappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		Fail_Return_Void(context->Map(perframeBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));

		CB_PerFrame* pConstants = (CB_PerFrame*)MappedResource.pData;
		
		pConstants->world = world_;
		pConstants->view =  v;
		pConstants->project = p;
		
		context->Unmap(perframeBuffer_, 0);

		//envmap
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		memset(&MappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		Fail_Return_Void(context->Map(envmapBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		CB_EnvMap* envmap = (CB_EnvMap*)MappedResource.pData;
		XMStoreFloat4x4(&envmap->invProjView, invProjView);
		envmap->eyePos = camara_.GetEyePos();
		context->Unmap(envmapBuffer_, 0);
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);
		ID3D11RenderTargetView* mainRT = this->GetMainRT();
		ID3D11DepthStencilView* mainDSV = this->GetMainDSV();

	
		ID3D11RenderTargetView* gbuffer[2] = { colorRT_, depthRT_ };
		ClearRenderTargets(2, gbuffer);

		context->OMSetRenderTargets(2, gbuffer, mainDSV);
		context->VSSetConstantBuffers(0, 1, &perframeBuffer_);
		context->PSSetConstantBuffers(0, 1, &perframeBuffer_);
		context->PSSetShaderResources(0, 1, &envTexture_);		
		context->VSSetShader(commonInputVertexPass_, nullptr, 0);
		context->PSSetShader(commonInputPixelPass_, nullptr, 0);
		ball_->Render(context);

		context->OMSetRenderTargets(1, &mainRT, nullptr);
		ID3D11ShaderResourceView* two[3] = { envTexture_, colorSRV_, depthSRV_ };
		context->PSSetShaderResources(0, 3, two);
		context->VSSetShader(fullScreenVertex_, nullptr, 0);
		context->PSSetShader(fullScreenPixel_, nullptr, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->Draw(3, 0);


		ID3D11ShaderResourceView*    pSRV[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		context->PSSetShaderResources(0, 8, pSRV);
	};

	virtual void MsgProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		camara_.ProcessMessage(hwnd, msg, wparam, lparam);
	}

	virtual void UpdateScene(float elapse)
	{
		camara_.Update(elapse);
	}

	virtual void Exit()
	{
	};
private:
	ID3D11VertexShader*  fullScreenVertex_;
	ID3D11PixelShader*   fullScreenPixel_;
	ID3D11VertexShader*  commonInputVertexPass_;
	ID3D11InputLayout*   commonInputLayout_;
	ID3D11PixelShader*   commonInputPixelPass_;
	
	ID3D11Buffer*        perframeBuffer_;
	ID3D11Buffer*        envmapBuffer_;

	ID3D11ShaderResourceView *envTexture_;
	ID3D11SamplerState   *TriLinerSampler_;
	ID3D11SamplerState   *PointSampler_;
	ID3D11RasterizerState* resterState_;
	ModelRender          * ball_;

	ID3D11RenderTargetView* colorRT_;
	ID3D11ShaderResourceView *colorSRV_;
	ID3D11RenderTargetView* depthRT_;
	ID3D11ShaderResourceView *depthSRV_;



	FirstPersonCamara camara_;
	XMFLOAT4X4 world_, view_, project_;
	float aspect;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR pScmdline, int iCmdshow)
{
	ul::OpenConsoleAndDebugLeak();

	Log_Info("hello");
	//Utils::SetBreakPointAtMemoryLeak(154);
	Lession1_Frame  *app = new Lession1_Frame;
	if (!app)
	{
		return 0;
	}

	// Initialize and run the system object.
	if (app->Initialize(800, 600))
	{
		app->Run();
	}

	// Shutdown and release the system object.
	app->Shutdown();
	delete app;
	app = 0;

	return 0;
}

