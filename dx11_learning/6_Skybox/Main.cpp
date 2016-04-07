#include<Windows.h>
#include<memory>

#include"util/tools.h"
#include"Application.h"
#include"res_mgr.h"

#include<ty_model_loader.h>
#include<renderable.h>
#include<common_model_loader.h>
#include<environmentable.h>
#include<D3DX10Math.h>

using namespace ul;


struct CB_PerFrame
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 project;
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

		ModelReader reader;
		SModelData data;
		reader.Load("../res/ty_model/", "../res/ty_model/slj_zwshu0060_wb.model", true, true, data);
		tree_.Create(dev, data);
		ModelData_Free(data);

		skybox_.Create(dev, "../res/skybox/skyDiffuseHDR.dds", "../res/skybox/skySpecularHDR.dds");

		Null_Return_Void((
			skyVertex_ = mgr->CreateVertexShaderAndInputLayout(
			"skybox.hlsl", "VS_FillBuffer", "vs_5_0",
			G_Layout_VertexXyznuv, ARRAYSIZE(G_Layout_VertexXyznuv), &xyznuv_)
		));
		Null_Return_Void((skyPixel_ = mgr->CreatePixelShader("skybox.hlsl", "PS_FillBuffer", "ps_5_0")));


		Null_Return_Void((
			modelVertex_ = mgr->CreateVertexShaderAndInputLayout(
			"main.hlsl", "VS_FillBuffer", "vs_5_0",
			G_Layout_VertexXyznuv, ARRAYSIZE(G_Layout_VertexXyznuv), &xyznuv_)
			));
		Null_Return_Void((modelPixel_ = mgr->CreatePixelShader("main.hlsl", "PS_FillBuffer", "ps_5_0")));

		Null_Return_Void((perframeBuffer_ = mgr->CreateConstantBuffer(sizeof(CB_PerFrame))));

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


	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		context->IASetInputLayout(xyznuv_);
		context->RSSetState(resterState_);
		context->PSSetSamplers(0, 1, &TriLinerSampler_);		

		XMFLOAT4X4 tv = camara_.GetTransposeViewMatrix();
		XMFLOAT4X4 tp = camara_.GetTransposeProjectMatrix();
		XMFLOAT4X4 p =  camara_.GetProjectMatrix();
		XMFLOAT4X4 v =  camara_.GetViewMatrix();

		XMVECTOR det;
		XMMATRIX invProj = XMMatrixInverse(&det, XMLoadFloat4x4(&p) );
		XMMATRIX invView = XMMatrixInverse(&det, XMLoadFloat4x4(&v) );
		invProj = XMMatrixTranspose(invProj);
		invView = XMMatrixTranspose(invView);

		//mvp
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		memset(&MappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		Fail_Return_Void(context->Map(perframeBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));

		CB_PerFrame* pConstants = (CB_PerFrame*)MappedResource.pData;
		pConstants->world = world_;
		pConstants->view =  tv;
		pConstants->project = tp;
		context->Unmap(perframeBuffer_, 0);
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);
		ID3D11RenderTargetView* mainRT = this->GetMainRT();
		ID3D11DepthStencilView* mainDSV = this->GetMainDSV();
		context->OMSetRenderTargets(1, &mainRT, nullptr);

		context->VSSetShader(skyVertex_, nullptr, 0);
		context->PSSetShader(skyPixel_, nullptr, 0);
		context->VSSetConstantBuffers(0, 1, &perframeBuffer_);
		skybox_.Render(context);

		context->OMSetRenderTargets(1, &mainRT, mainDSV);
		context->VSSetShader(modelVertex_, nullptr, 0);
		context->PSSetShader(modelPixel_, nullptr, 0);
		context->VSSetConstantBuffers(0, 1, &perframeBuffer_);
		tree_.Render(context);

		ID3D11ShaderResourceView*    pSRV[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		context->PSSetShaderResources(0, 8, pSRV);
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

	SkyBox                skybox_;
	BaseModel             tree_;

	FirstPersonCamara camara_;
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

