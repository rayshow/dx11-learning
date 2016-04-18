#include<Windows.h>
#include<memory>

#include<Application.h>
#include<D3DX10Math.h>
using namespace ul;


struct CB_PerFrame
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 worldViewProject;
	XMFLOAT4   coeffs[9];
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
		mgr->SetResourceBasePath("../res/");

		//camara
		pCamara_ = this->GetSceneMgr().GetMainCamara();
		pCamara_->LookAt(XMFLOAT4(0, 0, -100, 0), XMFLOAT4(0, 0, 0, 0));
		camaraController_.SetCamara(pCamara_);
		camaraController_.SetRotateAndMoveScaler(1, 20);

		pCamara_->LookAt(XMFLOAT4(0, 0, 0, 0), XMFLOAT4(0, 0, 1, 0));

		Null_Return_Void((envTexture_ = mgr->CreateTextureFromFile("../Res/skybox/sky1/domeSpecularHDR.dds")));
		Null_Return_Void((perframeBuffer_ = mgr->CreateConstantBuffer(sizeof(CB_PerFrame))));
		Null_Return_Void((ball_ = mgr->CreateModelFromFile("../res/mesh/sphere.x")));

		D3D11_SAMPLER_DESC SamDesc;
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

		context->PSSetSamplers(eSampler_EnvCubemap, 1, &TriLinerSampler_);

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
		pCamara_->SetProject(BaseCamara::eCamara_Perspective, XM_PI / 4, aspect, 0.1f, 1000.0f);
	}

	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		context->IASetInputLayout(commonInputLayout_);
		context->RSSetState(resterState_);
		context->PSSetSamplers(0, 1, &TriLinerSampler_);

		//mvp
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		memset(&MappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		Fail_Return_Void(context->Map(perframeBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));

		CB_PerFrame* pConstants = (CB_PerFrame*)MappedResource.pData;
		
		pConstants->world = world_;
		pConstants->worldViewProject = pCamara_->GetViewProjectStoreType();
		
		context->Unmap(perframeBuffer_, 0);

	
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);
		ID3D11RenderTargetView* mainRT = this->GetMainRT();
		ID3D11DepthStencilView* mainDSV = this->GetMainDSV();

		context->OMSetRenderTargets(1, &mainRT, mainDSV);

		ball_->SetConstBuffer(perframeBuffer_);
		ball_->SetShaderResource(eShaderResource_EnvCubemap, envTexture_);
		ball_->Render(context);

		ID3D11ShaderResourceView*    pSRV[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		context->PSSetShaderResources(0, 8, pSRV);
	};

	virtual int MsgProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		camaraController_.ProcessMessage(hwnd, msg, wparam, lparam);
		return 0;
	}

	virtual void UpdateScene(float elapse)
	{
		camaraController_.Update(elapse);
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
	BaseModel           * ball_;

	ID3D11RenderTargetView*  colorRT_;
	ID3D11ShaderResourceView *colorSRV_;
	ID3D11RenderTargetView*  depthRT_;
	ID3D11ShaderResourceView *depthSRV_;

	FirstPersonController camaraController_;
	BaseCamara*           pCamara_;


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

