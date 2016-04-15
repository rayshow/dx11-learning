#include<Windows.h>
#include<memory>

#include<Application.h>

#include<D3DX10Math.h>

using namespace ul;

struct CB_PerFrame
{
	XMFLOAT4X4 rotateProject;
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

		pCamara_ = SceneMgr::GetSingletonPtr()->GetMainCamara();
		pCamara_->LookAt(XMFLOAT4(0, 0, 0, 0), XMFLOAT4(0, 0, 1, 0));
		controller.SetCamara(pCamara_);

		environment_ = SceneMgr::GetSingletonPtr()->GetEnvironment();
		environment_->LoadFromFile("skybox/sky1/dome1.env");
		skybox_.Create(dev, environment_);

		perframeBuffer_ = mgr->CreateConstantBuffer(sizeof(CB_PerFrame));

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
		SamDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		Null_Return_Void((LinerSampler_ = mgr->CreateSamplerState(SamDesc)));

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
		pCamara_->SetProject(BaseCamara::eCamara_Perspective, XM_PI / 4, aspect, 0.1f, 1000.0f);
	};


	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		context->RSSetState(resterState_);
		context->PSSetSamplers(0, 1, &TriLinerSampler_);		
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);
		ID3D11RenderTargetView* mainRT = this->GetMainRT();
		ID3D11DepthStencilView* mainDSV = this->GetMainDSV();

		context->OMSetRenderTargets(1, &mainRT, nullptr);
		skybox_.Render(context);

		ID3D11ShaderResourceView*    pSRV[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		context->PSSetShaderResources(0, 8, pSRV);
	};

	virtual int MsgProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		 controller.ProcessMessage(hwnd, msg, wparam, lparam);
		 return 0;
	}

	virtual void UpdateScene(float elapse)
	{
		controller.Update(elapse);
	}

	virtual void Exit()
	{
	};
private:
	ID3D11VertexShader*  skyVertex_;
	ID3D11PixelShader*   skyPixel_;
	ID3D11VertexShader*  modelVertex_;
	ID3D11PixelShader*   modelPixel_;

	ID3D11InputLayout*   xyznuv_;
	
	ID3D11Buffer*        perframeBuffer_;
	ID3D11Buffer*        envmapBuffer_;

	ID3D11SamplerState   *TriLinerSampler_;
	ID3D11SamplerState   *LinerSampler_;
	ID3D11SamplerState   *PointSampler_;

	ID3D11RasterizerState* resterState_;

	Environmentable*      environment_;
	SkyBox                skybox_;

	BaseCamara*           pCamara_;
	FirstPersonController controller;

	XMFLOAT4X4 world_, view_, project_;
	float aspect;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR pScmdline, int iCmdshow)
{
	//ul::SetBreakPointAtMemoryLeak(505);
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

