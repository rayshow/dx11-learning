#ifndef UL_POSTPROCESS_HEADER__
#define UL_POSTPROCESS_HEADER__

#include<string>

#include<d3d11.h>
#include<D3DX11tex.h>
#include"res_mgr.h"


namespace ul
{
	using namespace std;

	class PostProcess
	{
		struct SPostProcess_Parameter
		{
			float  mipLevel;
			float  padding[3];
		};

	private:
		ID3D11RenderTargetView*		renderTarget_;
		ID3D11ShaderResourceView*   shaderResource_;
		ID3D11VertexShader*         fullScreenVs_;
		ID3D11PixelShader*          fullScreenPs_;
		ID3D11Texture2D*            srvOwnerTexture_;
		ulUint                      width_;
		ulUint                      height_;

		DXGI_FORMAT                 format_;
		SPostProcess_Parameter      parameter_;
	public:
		PostProcess(){}
		~PostProcess(){}
	public:
		void SetShaderResourceView(ID3D11ShaderResourceView* shaderResourceView)
		{
			this->shaderResource_ = shaderResourceView;
		}

		bool Create(const string& fileName,
			const string& psFunction,
			DXGI_FORMAT& format,
			ulUint width, ulUint height, 
			ID3D11Texture2D *owner = nullptr,
			ulUint mipLevel = 0)
		{
			ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();

			width_  = width;
			height_ = height;
			format_ = format;
			parameter_.mipLevel = mipLevel;
			srvOwnerTexture_ = owner_;

			//from a exists texture2d
			if (!Null(owner))
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
				srvDesc.Texture2D.MipLevels = -1;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = format;
				Null_Return_False_With_Msg(
					(shaderResource_ = mgr->CreateShaderResourceView(owner, &srvDesc, ResourceMgr::eRecyclePerSwapChain)),
					"create postprocess from shader %s : %s error",
					fileName.c_str(),
					psFunction.c_str()
				);

			}
			else{
				//entirely single texture2d
				D3D11_TEXTURE2D_DESC texDesc;
				texDesc.Width = width;
				texDesc.Height = height;
				texDesc.MipLevels = 0;
				texDesc.ArraySize = 1;
				texDesc.Format = format;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.Usage = D3D11_USAGE_DEFAULT;
				texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				texDesc.CPUAccessFlags = 0;
				texDesc.MiscFlags = 0;

				mgr->Create

			}
			


			//mgr->CreateShaderResourceView()
		}
	

	};
};


#endif