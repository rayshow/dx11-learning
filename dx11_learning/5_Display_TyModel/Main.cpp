#include<Windows.h>
#include<memory>

#include<Application.h>
#include<resource/TyModelLoader.h>
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
		mgr->SetResourceBasePath("../res/");
		
		pCamara_ = SceneMgr::GetSingletonPtr()->GetMainCamara();
		pCamara_->LookAt(XMFLOAT4(0, 0, 0, 0), XMFLOAT4(0, 0, 1, 0));
		camaraController_.SetCamara(pCamara_);

		TyModelReader reader;
		SModelData data;
		reader.Load("../res/ty_model/", "../res/ty_model/slj_zwshu0060_wb.model", true, true, data);
		tree_.Create(dev, data);
		ModelData_Free(data);

		Null_Return_Void((samplers_[0] = mgr->CreateLinearSamplerState()));
		samplers_[1] = samplers_[2] = samplers_[0];
	};


	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		context->PSSetSamplers(0, 3, samplers_);
	
		XMFLOAT4X4 view = pCamara_->GetViewStoreType();
		XMFLOAT4X4 proj = pCamara_->GetProjectStoreType();

		perFrame_.world = world_;
		perFrame_.view = view;
		perFrame_.project = proj;
		ResourceMgr::GetSingleton().MappingBufferWriteOnly(perframeBuffer_, &perFrame_, sizeof(CB_PerFrame));
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);
		ID3D11RenderTargetView* mainRT = this->GetMainRT();
		ID3D11DepthStencilView* mainDSV = this->GetMainDSV();
		context->OMSetRenderTargets(1, &mainRT, mainDSV);

		tree_.SetConstBuffer(perframeBuffer_);
		tree_.Render(context);
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
	ID3D11VertexShader*  modelVertex_;
	ID3D11PixelShader*   modelPixel_;
	ID3D11InputLayout*   xyznuvtbwwiii_;
	
	ID3D11Buffer*        perframeBuffer_;
	ID3D11Buffer*        envmapBuffer_;

	ID3D11SamplerState   *samplers_[3];
	CB_PerFrame           perFrame_;
	BaseModel             tree_;
	FirstPersonController camaraController_;
	BaseCamara*           pCamara_;
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

