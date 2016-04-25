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
	protected:
		int						   width_;
		int						   height_;
		string					   appName_;
		bool					   fullscreen_;
		HWND					   hWnd_;
		HINSTANCE				   hInstance_;
		Timer					   timer_;
		bool					   running_;
		char					   fpsDisplay_[20];
		D3D11GraphicsContext       graphicsContext_;
		ResourceMgr                resourceMgr_;
		SceneMgr				   sceneMgr_;
	
		bool                       resizing_;
		bool                       minimized_;
		bool                       maximized_;
		ulUint                     clientWidth_;
		ulUint                     clientHeight_;
	public:
		Application() :
			width_(0),
			height_(0),
			appName_("Dx11Appcalition"),
			fullscreen_(false),
			running_(false),
			resizing_(false),
			minimized_(false),
			maximized_(false)
		{
			memset(fpsDisplay_, 0, 20);
			Log_Info("application construct.");
		}
	    ~Application(){};

		void Shutdown()
		{
			resourceMgr_.ReleaseLoadedResourceOnResize();
			resourceMgr_.ReleaseLoadedResourceOnExit();
			sceneMgr_.Shutdown();
			graphicsContext_.Shutdown();
			this->Exit();
			Log_Info("application destoryed.");
		}

	public:
		HWND GetHwnd() { return hWnd_; }

		SceneMgr& GetSceneMgr() { return sceneMgr_; }

		float GetFPS() { return timer_.GetFPS(); }

		void SetResourceBasePath(string path){ resourceMgr_.SetResourceBasePath(path);  }

		void SetAppcationName(const string& name) { appName_ = name; }

		float GetElapse() 	{ return timer_.GetDeltaTime(); }

		ID3D11Device* GetDevicePtr() { return graphicsContext_.GetDevicePtr(); }

		ID3D11DeviceContext* GetDeviceContextPtr() { return graphicsContext_.GetDeviceContextPtr(); }

		ID3D11RenderTargetView* GetMainRT() { return graphicsContext_.GetMainRenderTargetPtr(); }

		ID3D11DepthStencilView* GetMainDSV() { return graphicsContext_.GetMainDepthStencilViewPtr(); }

		void ClearRenderTarget(ID3D11RenderTargetView* rt) { GetDeviceContextPtr()->ClearRenderTargetView(rt, BLACK_COLOR); }

		void ClearRenderTargets(int num, ID3D11RenderTargetView** rt)
		{
			for (int i = 0; i < num; ++i)
			{
				ClearRenderTarget(rt[i]);
			}
		}


		bool Initialize(int width, int height)
		{
			this->width_  = width;
			this->height_ = height;

			//init window
			fullscreen_ = false;
			this->initializeWindow(width, height);
		
			//init graphics
			False_Return_False(graphicsContext_.Initialize( &resourceMgr_, hWnd_,
				clientWidth_, clientHeight_, true, fullscreen_, false));

			//init scene manager
			False_Return_False( sceneMgr_.Initialize(&resourceMgr_) );

			//call InitResource 
			False_Return_False( this->InitResource( GetDevicePtr(), GetDeviceContextPtr() ) );
			
			// when initialized show window
			ShowWindow(hWnd_, SW_SHOW);
			SetForegroundWindow(hWnd_);
			SetFocus(hWnd_);

		

			running_ = true;
			Log_Info("application %s initialized.", appName_.c_str());
			return true;
		}

		void Run()
		{
			MSG msg;
			while (true)
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

		LRESULT CALLBACK MsgHandle(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			switch (msg)
			{
			case WM_ENTERSIZEMOVE:
				resizing_ = true;
				break;
			case WM_EXITSIZEMOVE:
				resizing_ = false;
				resize();
				break;
			case WM_SIZE:
				clientWidth_ = LOWORD(lparam);
				clientHeight_ = HIWORD(lparam);
				switch (wparam)
				{
				case SIZE_MINIMIZED:
					minimized_ = true;
					maximized_ = false;
					break;
				case SIZE_MAXIMIZED:
					minimized_ = false;
					maximized_ = true;
					resize();
					break;
				case SIZE_RESTORED:
					if(!resizing_) resize();
					break;
				}
				break;
			}
	
			return this->MsgProcess(hwnd, msg, wparam, lparam);
		}

	protected:
		virtual void WindowResize(
			int width, int height, ID3D11Device *dev,
			ID3D11DeviceContext* context) = 0;

		virtual bool InitResource(
			ID3D11Device *dev,
			ID3D11DeviceContext* context) = 0;

		virtual void RenderFrame(
			ID3D11Device *dev,
			ID3D11DeviceContext* context) = 0;

		virtual int MsgProcess(
			HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) = 0;

		virtual void Exit() = 0;

		virtual void UpdateScene(float elapse) = 0;

	private:
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

		void resize()
		{
			graphicsContext_.Resize(clientWidth_, clientHeight_);
			this->WindowResize(clientWidth_, clientHeight_, GetDevicePtr(), GetDeviceContextPtr());
		}
	};

};
#endif