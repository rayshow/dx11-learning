#ifndef DX11_APPLICATION_HEADER__
#define DX11_APPLICATION_HEADER__

#include<Windows.h>
#include<string>

#include"util/singleton.h"
#include"Graphics.h"
#include"Input.h"
#include"Camara.h"
#include"util/timer.h"

using std::string;


namespace ul
{
	class Application :public Singleton<Application>
	{
	private:
		int       width_;
		int       height_;
		string    appName_;
		bool      fullscreen_;
		Input	  input_;
		Graphics  graphics_;
		HWND      hWnd_;
		HINSTANCE hInstance_;
		Timer     timer_;

	public:
		Application() :
			width_(0),
			height_(0),
			appName_("Dx11Appcalition"),
			fullscreen_(false)
		{
			s_Singleton = this;
			Log_Info("application construct.");
		}
		virtual ~Application(){
			Log_Info("application destoryed.");
		};

	public:
		void SetAppcationName(const string& name)
		{
			appName_ = name;
		}

		bool Initialize(int width, int height)
		{
			this->width_  = width;
			this->height_ = height;

			fullscreen_ = false;
			this->initializeWindow(width, height);
			input_.Initialize();
			graphics_.Initialize(width, height, hWnd_, true, fullscreen_);

			this->InitResource( GetDevicePtr(), GetDeviceContextPtr() );
		
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

		LRESULT CALLBACK InputProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			this->MsgProcess(hwnd, msg, wparam, lparam);
			switch (msg)
			{
				case WM_KEYDOWN:
				{
					input_.KeyDown(wparam);
					return 0;
				}
				case WM_KEYUP:
				{
					input_.KeyUp(wparam);
					return 0;
				}
			}
		}

		void Shutdown()
		{
			graphics_.Shutdown();
			this->Exit();
		}

		void OnResize(int width, int height)
		{
			graphics_.GetD3D().Resize(width, height);
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

		virtual void MsgProcess(
			HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) = 0;

		virtual void Exit() = 0;

		virtual void UpdateScene(float elapse) = 0;

	private:
		bool initializeWindow(int width, int height);
		bool frame(){ 
			timer_.Frame();
			this->UpdateScene(timer_.GetElapsedSeconds());

			graphics_.BeginRender();
			this->RenderFrame(GetDevicePtr(), GetDeviceContextPtr());
			graphics_.EndRender();

			return true; 
		};
		ID3D11Device* GetDevicePtr()
		{
			return graphics_.GetD3D().GetDevicePtr();
		}
		ID3D11DeviceContext* GetDeviceContextPtr()
		{
			return graphics_.GetD3D().GetDeviceContextPtr();
		}
		ID3D11RenderTargetView* GetMainRT()
		{
			return graphics_.GetD3D().GetMainRenderTargetPtr();
		}

		ID3D11DepthStencilView* GetMainDSV()
		{
			return graphics_.GetD3D().GetMainDepthStencilViewPtr();
		}
	};

};
#endif