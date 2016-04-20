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

	virtual void InitResource(
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		ResourceMgr *mgr = ResourceMgr::GetSingletonPtr();
		mgr->SetResourceBasePath("../res/");

		pCamara_ = SceneMgr::GetSingletonPtr()->GetMainCamara();
		pCamara_->LookAt(XMFLOAT4(0, 0, 0, 0), XMFLOAT4(0, 0, 1, 0));
		controller.SetCamara(pCamara_);
		skybox_.Create(dev, "skybox/sky1/dome1.env");
	};

	virtual void WindowResize(int width, int height,
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		ResourceMgr *mgr = ResourceMgr::GetSingletonPtr();
		mgr->ReleaseLoadedResourcePerSwapChain();
		aspect = (float)width / (float)height;
		pCamara_->SetProject(BaseCamara::eCamara_Perspective, XM_PI / 4, aspect, 0.1f, 1000.0f);

		skybox_.ApplySkyBox(context);
	};


	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		context->RSSetState(resterState_);
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);
		ID3D11RenderTargetView* mainRT = this->GetMainRT();
		ID3D11DepthStencilView* mainDSV = this->GetMainDSV();

		context->OMSetRenderTargets(1, &mainRT, nullptr);
		skybox_.Render(context);
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
	ID3D11SamplerState   *TriLinerSampler_;
	ID3D11SamplerState   *LinerSampler_;
	ID3D11SamplerState   *PointSampler_;

	ID3D11RasterizerState* resterState_;

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

