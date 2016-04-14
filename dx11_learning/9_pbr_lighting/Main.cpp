#include<Windows.h>
#include<memory>

#include"util/tools.h"
#include"Application.h"
#include"res_mgr.h"

#include<ty_model_loader.h>
#include<renderable.h>
#include<common_model_loader.h>
#include<environmentable.h>
#include<AntTweakBar.h>
#include<postprocess.h>

#include<xnamath.h>

using namespace ul;


struct CB_PerFrame
{
	XMFLOAT4X4  world;
	XMFLOAT4X4  view;
	XMFLOAT4X4  project;
	XMFLOAT3    camaraPos;
	float       padding;
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

		uiRotate_.w = 1;
		uiLightDir_.x = 1;
		XMVECTOR axis = XMLoadFloat4(&XMFLOAT4(1, 0, 0, 0));
		XMVECTOR rotate = XMQuaternionRotationAxis(axis, -XM_PI / 2);
		XMStoreFloat4(&uiRotate_, rotate);

		//ui
		TwInit(TW_DIRECT3D11, this->GetDevicePtr());
		TwBar *bar = TwNewBar("TweakBar");
		TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar into a DirectX11 application.' "); 
		int barSize[2] = { 224, 320 };
		TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
		TwAddVarRW(bar, "Level", TW_TYPE_INT32, &uiLevel_, "min=0 max=100 group=Sponge keyincr=l keydecr=L");
		TwAddVarRW(bar, "Animate", TW_TYPE_BOOLCPP, &uiAnim_, "group=Sponge key=o");
		TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &uiRotate_, "opened=true axisz=-z group=Sponge");
		TwAddVarRW(bar, "Light direction", TW_TYPE_DIR3F, &uiLightDir_, "opened=true axisz=-z showval=false");
		TwAddVarRW(bar, "Camera distance", TW_TYPE_FLOAT, &uiCamaraDistance_, "min=0 max=4 step=0.01 keyincr=PGUP keydecr=PGDOWN");
		TwAddVarRW(bar, "Background", TW_TYPE_COLOR4F, &uiLightColor_, "colormode=hls");


		//camara
		pCamara_ = this->GetSceneMgr().GetMainCamara();
		pCamara_->LookAt(XMFLOAT4(0, 0, -100, 0), XMFLOAT4(0, 0, 0, 0));
		camaraController_.SetCamara(pCamara_);

		//environment, skybox
		environment_ = this->GetSceneMgr().GetEnvironment();
		environment_->LoadFromFile("skybox/sky1/dome1.env");
		skybox_.Create(dev, environment_);


		//model
		pistol_ = mgr->CreateModelFromFile("pbr_model/pistol/pistol.fbx");
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

	virtual void WindowResize(int width, int height,
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		ResourceMgr *mgr = ResourceMgr::GetSingletonPtr();
		mgr->ReleaseLoadedResourcePerSwapChain();

		aspect = (float)width / (float)height;
		XMStoreFloat4x4(&world_, XMMatrixIdentity());
		pCamara_->SetProject(BaseCamara::eCamara_Perspective, XM_PI / 4, aspect, 0.1f, 1000.0f);

		postProcessChain_.Create(width, height);
		//postProcessChain_.AddPostProcess(PostProcessChain::ePostProcess_PresentHDR);

	};


	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		context->IASetInputLayout(xyznuvtbwwiii_);
		context->RSSetState(resterState_);
		ID3D11SamplerState* state[2] = { LinerSampler_, TriLinerSampler_ };
		context->PSSetSamplers(0, 2, state);

		XMVECTOR rotQuaternion = XMLoadFloat4(&uiRotate_);
		XMMATRIX rotMatrix = XMMatrixRotationQuaternion(rotQuaternion);
		rotMatrix = XMMatrixTranspose(rotMatrix);
		XMStoreFloat4x4(&world_, rotMatrix);

		XMFLOAT4X4 tv =  pCamara_->GetTransposeViewMatrix();
		XMFLOAT4X4 tp =  pCamara_->GetTransposeProjectMatrix();
		XMFLOAT4X4 p =	 pCamara_->GetProjectMatrix();
		XMFLOAT4X4 v =   pCamara_->GetViewMatrix();
		XMFLOAT4   pos = pCamara_->GetEyePos();

		XMVECTOR det;
		XMMATRIX invProj = XMMatrixInverse(&det, XMLoadFloat4x4(&p) );
		XMMATRIX invView = XMMatrixInverse(&det, XMLoadFloat4x4(&v) );
		invProj = XMMatrixTranspose(invProj);
		invView = XMMatrixTranspose(invView);

		
		CB_PerFrame perFrame;
		perFrame.world = world_;
		perFrame.view = tv;
		perFrame.project = tp;
		perFrame.camaraPos = XMFLOAT3(pos.x, pos.y, pos.z);
		ResourceMgr::GetSingletonPtr()->MappingBufferWriteOnly(perframeBuffer_, &perFrame, sizeof(CB_PerFrame));

	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);
		ID3D11RenderTargetView* mainRT = this->GetMainRT();
		ID3D11DepthStencilView* mainDSV = this->GetMainDSV();

		////sky box
		//context->OMSetRenderTargets(1, &mainRT, nullptr);

		postProcessChain_.BindAsRenderTarget(context, nullptr);
		context->VSSetConstantBuffers(0, 1, &perframeBuffer_);
		skybox_.Render(context);

		//obj
		postProcessChain_.BindAsRenderTarget(context, mainDSV);
		context->VSSetConstantBuffers(0, 1, &perframeBuffer_);
		context->PSSetConstantBuffers(0, 1, &perframeBuffer_);
		pistol_->Render(context);

		//postProcessChain_.Process(context);
		postProcessChain_.Present(context, mainRT);

		//ui
		TwDraw();

		ID3D11ShaderResourceView*    pSRV[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		context->PSSetShaderResources(0, 8, pSRV);
	};

	virtual int MsgProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		// Send event message to AntTweakBar
		if (TwEventWin(hwnd, msg, wparam, lparam))
		{
			return 0;
		}
		camaraController_.ProcessMessage(hwnd, msg, wparam, lparam);
	}

	virtual void UpdateScene(float elapse)
	{
		camaraController_.Update(elapse);
	}

	virtual void Exit()
	{
		TwTerminate();
	};
private:
	ID3D11VertexShader*  skyVertex_;
	ID3D11PixelShader*   skyPixel_;
	ID3D11VertexShader*  modelVertex_;
	ID3D11PixelShader*   modelPixel_;
	ID3D11VertexShader*  hdrPresentVs_;
	ID3D11PixelShader*   hdrPresentPs_;

	ID3D11InputLayout*   xyznuvtbwwiii_;
	ID3D11InputLayout*   xyznuv_;
	ID3D11Buffer*        perframeBuffer_;
	ID3D11Buffer*        envmapBuffer_;

	ID3D11SamplerState   *TriLinerSampler_;
	ID3D11SamplerState   *LinerSampler_;
	ID3D11SamplerState   *PointSampler_;

	ID3D11RasterizerState* resterState_;

	PostProcessChain      postProcessChain_;
	Renderable*           pistol_;
	SkyBox                skybox_;
	Environmentable*      environment_;
	FirstPersonController camaraController_;
	BaseCamara*           pCamara_;
	XMFLOAT4X4			  world_, view_, project_;
	float				  aspect;
	
	int                   uiLevel_;
	int                   uiAnim_;
	XMFLOAT4              uiRotate_;
	XMFLOAT4              uiLightDir_;
	float                 uiCamaraDistance_;
	XMFLOAT4              uiLightColor_;
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

