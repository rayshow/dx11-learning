#ifndef DX11_APPLICATION_HEADER__
#define DX11_APPLICATION_HEADER__

#include<Windows.h>
#include<string>

#include"design_frame/Singleton.h"
#include"util/UlHelper.h"
#include"util/Timer.h"
#include"scene/SceneManager.h"
#include"D3D11GraphicContext.h"


using std::string;

namespace ul
{
	const static float BLACK_COLOR[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	class Application :public Singleton<Application>
	{
	private:
		int       width_;
		int       height_;
		string    appName_;
		bool      fullscreen_;
		HWND      hWnd_;
		HINSTANCE hInstance_;
		Timer     timer_;
		bool      initialized_;
		char      fpsDisplay_[20];
		D3D11GraphicsContext       graphicsContext_;
		SceneMgr  sceneMgr_;
	public:
		Application() :
			width_(0),
			height_(0),
			appName_("Dx11Appcalition"),
			fullscreen_(false),
			initialized_(false)
		{
			s_Singleton = this;
			memset(fpsDisplay_, 0, 20);
			Log_Info("application construct.");
		}
		virtual ~Application(){
			Log_Info("application destoryed.");
		};

	public:
		HWND GetHwnd()
		{
			return hWnd_;
		}

		SceneMgr& GetSceneMgr()
		{
			return sceneMgr_;
		}

		void SetAppcationName(const string& name)
		{
			appName_ = name;
		}

		bool Initialize(int width, int height)
		{
			this->width_  = width;
			this->height_ = height;

			//init window
			fullscreen_ = false;
			this->initializeWindow(width, height);
			
			//init graphics
			RECT rect;
			GetClientRect(hWnd_, &rect);
			graphicsContext_.Initialize(hWnd_, rect.right-rect.left, rect.bottom-rect.top, true, fullscreen_, false);

			//call InitResource 
			this->InitResource( GetDevicePtr(), GetDeviceContextPtr() );
			

			// when initialized show window
			ShowWindow(hWnd_, SW_SHOW);
			SetForegroundWindow(hWnd_);
			SetFocus(hWnd_);

			initialized_ = true;
			Log_Info("application %s initialized.", appName_.c_str());

			return true;
		}

		void Run()
		{
			MSG msg;
			while (initialized_)
			{
				if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				if (msg.message == WM_QUIT || false == frame())
				{
					break;
				}
			}
		}

		LRESULT CALLBACK InputProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			//lost device and reset
			if (msg == WM_SIZE)
			{
				int nWidth = LOWORD(lparam); // width of client area
				int nHeight = HIWORD(lparam); // height of client area
				if (nWidth >= 1 && nHeight >= 1)
					this->OnResize(nWidth, nHeight);
			}			
			return this->MsgProcess(hwnd, msg, wparam, lparam);
		}

		void Shutdown()
		{
			graphicsContext_.Shutdown();
			this->Exit();
		}

		void OnResize(int width, int height)
		{
			graphicsContext_.Resize(width, height);
			this->WindowResize(width, height, GetDevicePtr(), GetDeviceContextPtr());
		}

		float GetFPS()
		{
			return timer_.GetFPS();
		}

		float GetElapse()
		{
			return timer_.GetElapsedSeconds();
		}
	public:
		virtual void WindowResize(
			int width, int height, ID3D11Device *dev,
			ID3D11DeviceContext* context) = 0;

		virtual void InitResource(
			ID3D11Device *dev,
			ID3D11DeviceContext* context) = 0;

		virtual void RenderFrame(
			ID3D11Device *dev,
			ID3D11DeviceContext* context) = 0;

		virtual int MsgProcess(
			HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) = 0;

		virtual void Exit() = 0;

		virtual void UpdateScene(float elapse) = 0;

	protected:
		bool initializeWindow(int width, int height);
		bool frame(){ 
			sprintf(fpsDisplay_, "fps:%f ", this->GetFPS());
			::SetWindowText(hWnd_, fpsDisplay_);
			timer_.Tick();
			this->UpdateScene(timer_.GetDeltaTime());

			graphicsContext_.BeginScene();
			this->RenderFrame(GetDevicePtr(), GetDeviceContextPtr());
			graphicsContext_.EndScene();

			return true; 
		};
		ID3D11Device* GetDevicePtr()
		{
			return graphicsContext_.GetDevicePtr();
		}
		ID3D11DeviceContext* GetDeviceContextPtr()
		{
			return graphicsContext_.GetDeviceContextPtr();
		}
		ID3D11RenderTargetView* GetMainRT() 
		{
			return graphicsContext_.GetMainRenderTargetPtr();
		}

		ID3D11DepthStencilView* GetMainDSV()
		{
			return graphicsContext_.GetMainDepthStencilViewPtr();
		}

		void ClearRenderTarget(ID3D11RenderTargetView* rt)
		{
			GetDeviceContextPtr()->ClearRenderTargetView(rt, BLACK_COLOR);
		}

		void ClearRenderTargets(int num, ID3D11RenderTargetView** rt)
		{
			for (int i = 0; i < num; ++i)
			{
				ClearRenderTarget(rt[i]);
			}
		}
	};

};
#endif