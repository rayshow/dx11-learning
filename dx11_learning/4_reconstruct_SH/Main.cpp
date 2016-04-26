#include<Windows.h>
#include<memory>

#include<Application.h>
#include<D3DX10Math.h>
using namespace ul;

class Lession1_Frame :public Application
{
public:
	virtual ~Lession1_Frame(){}
public:

	virtual bool InitResource(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		//camara
		pCamara_ = sceneMgr_.GetMainCamaraPtr();
		camaraController_.SetCamara(pCamara_);
		camaraController_.SetRotateAndMoveScaler(1, 20);

		pCamara_->LookAt(XMFLOAT4(0, 0, 0, 0), XMFLOAT4(0, 0, 1, 0));
		Null_Return_Void((ball_ = resourceMgr_.CreateStaticMeshRenderFromFile("mesh/sphere.x")));
	};


	virtual void WindowResize(
		int width, int height,
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		aspect = (float)width / (float)height;
		pCamara_->SetProject(BaseCamara::eCamara_Perspective, XM_PI / 4, aspect, 0.1f, 1000.0f);
	}

	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		ball_->Render(context);
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
	StaticMeshRender		 *ball_;
	FirstPersonController camaraController_;
	BaseCamara*           pCamara_;
	float aspect;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR pScmdline, int iCmdshow)
{
	ul::OpenConsoleAndDebugLeak();
	Log_Info("hello");

	//Utils::SetBreakPointAtMemoryLeak(154);
	Lession1_Frame  app;

	// Initialize and run the system object.
	if (app.Initialize(800, 600))
	{
		app.Run();
	}

	// Shutdown and release the system object.
	app.Shutdown();
	
	return 0;
}

