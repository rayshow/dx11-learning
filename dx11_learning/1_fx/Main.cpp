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


class Lession1_Frame :public Application
{
public:
	virtual ~Lession1_Frame(){}
public:

	virtual bool InitResource(
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		//bg
		sceneMgr_.SetEnvironment("skybox/sky1/dome1.env");

		//camara
		pCamara_ = sceneMgr_.CreateCamara();
		sceneMgr_.SetMainCamara(pCamara_);
		pCamara_->LookAt(XMFLOAT4(0, 0, -10, 0), XMFLOAT4(0, 0, 0, 0));
		controller.SetCamara(pCamara_);
		controller.SetRotateAndMoveScaler(2, 10);

		//gun
		pistol_ = sceneMgr_.CreateStaticObject("pbr_model/pistol/pistol.fbx");
		//pistol_->SetEffect("test.fxo");

		return true;
	};

	virtual void WindowResize(int width, int height,
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		aspect_ = (float)width / (float)height;
		pCamara_->SetProject(BaseCamara::eCamara_Perspective, XM_PI / 4, aspect_, 0.1f, 10000.0f);
	};

	virtual void RenderFrame(
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		sceneMgr_.RenderAll(context);
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
	BaseCamara*					 pCamara_;
	SkyBox*                      skyBox_;
	FirstPersonController		 controller;
	StaticMeshRender*            pistol_;
	
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR pScmdline, int iCmdshow)
{
	//ul::SetBreakPointAtMemoryLeak(542);
	ul::OpenConsoleAndDebugLeak();	
	Log_Info("hello");
	//Utils::SetBreakPointAtMemoryLeak(154);


	Lession1_Frame  app;
	// Initialize and run the system object.

	app.SetResourceBasePath("../res/");
	if (app.Initialize(1024, 768))
	{
		app.Run();
	}
	app.Shutdown();

	return 0;
}

