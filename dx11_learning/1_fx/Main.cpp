#include<Windows.h>
#include<memory>

#include<Application.h>
#include<D3DX10Math.h>
#include<d3dx11effect.h>
#include<iostream>
#include<ios>
#include<fstream>

using namespace ul;
using namespace std;

struct CB_PerFrame
{
	XMFLOAT4X4 rotateProject;
};

class Lession1_Frame :public Application
{
public:
	virtual ~Lession1_Frame(){}
public:

	virtual bool InitResource(
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		ResourceMgr *mgr = ResourceMgr::GetSingletonPtr();
		mgr->SetResourceBasePath("../res/");

		pCamara_ = SceneMgr::GetSingletonPtr()->GetMainCamara();
		pCamara_->LookAt(XMFLOAT4(0, 0, 200, 0), XMFLOAT4(0, 0, 1, 0));
		controller.SetCamara(pCamara_);

		Null_Return_False((pPistolRender_ = mgr->CreateStaticMeshRenderFromFile("pbr_model/pistol/pistol.fbx")));
		Null_Return_False((testFx_ = mgr->LoadEffectFromCompileFile("test.fxo")));
		Null_Return_False((wvp_ = testFx_->GetVariableByName("WorldViewProject")->AsMatrix()));
		pPistolRender_->SetEffect("test.fx");
		return true;
	};

	virtual void WindowResize(int width, int height,
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		aspect_ = (float)width / (float)height;
		pCamara_->SetProject(BaseCamara::eCamara_Perspective, XM_PI / 4, aspect_, 0.1f, 1000.0f);
	};


	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		wvp_->SetMatrix( pCamara_->GetViewProjectStorePtr() );
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);
		ID3D11RenderTargetView* mainRT = this->GetMainRT();
		context->OMSetRenderTargets(1, &mainRT, this->GetMainDSV());
		pPistolRender_->Render(context);
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
	float						 aspect_;
	ID3DX11Effect*				 testFx_;
	ID3DX11EffectMatrixVariable* wvp_;
	BaseCamara*					 pCamara_;
	FirstPersonController		 controller;
	StaticMeshRender*            pPistolRender_;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR pScmdline, int iCmdshow)
{
	//ul::SetBreakPointAtMemoryLeak(505);
	ul::OpenConsoleAndDebugLeak();	
	Log_Info("hello");
	//Utils::SetBreakPointAtMemoryLeak(154);


	Lession1_Frame  app;
	// Initialize and run the system object.
	if (app.Initialize(800, 600))
	{
		app.Run();
	}
	app.Shutdown();

	return 0;
}

