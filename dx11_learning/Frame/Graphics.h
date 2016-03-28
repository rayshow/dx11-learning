#ifndef GRAPHICS_HEADER__
#define GRAPHICS_HEADER__

#include"util/tools.h"
#include"D3D.h"

namespace ul
{
	class Graphics
	{
	private:
		D3D    d3d_;

	public:
		bool Initialize(int width, int height, HWND hwnd, bool vsyn, bool fullscreen)
		{
			False_Return_False_With_Msg(
				d3d_.Initialize(hwnd, width, height, vsyn, fullscreen),
				"d3d initalize failed!");
			return true;
		}
		void Shutdown()
		{
			d3d_.Shutdown();
		}
		void BeginRender()
		{
			d3d_.BeginScene();
		}
		void EndRender()
		{
			d3d_.EndScene();
		}
		D3D& GetD3D()
		{
			return d3d_;
		}
	};
}





#endif