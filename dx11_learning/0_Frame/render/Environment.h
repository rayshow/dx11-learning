#ifndef UL_SKYBOX_HEADER__
#define UL_SKYBOX_HEADER__

#include<string>
#include<d3d11.h>
#include<xnamath.h>

#include"MeshRender.h"
#include"../D3D11GraphicContext.h"

namespace ul
{
	class Environment
	{
	protected:
		ID3D11ShaderResourceView*  environmentMaps_[3];
		ID3D11SamplerState*        samplers_[3];
		ulFloat                    exposure_;
	public:
		Environment()
		{
			memset(environmentMaps_, 0, sizeof(ID3D11ShaderResourceView*) * 3);
			memset(samplers_, 0, sizeof(ID3D11SamplerState*) * 3);
		}
		~Environment(){}
		bool Initialize(const string& fileName);


		ID3D11ShaderResourceView** GetEnvironmentmaps()
		{
			return environmentMaps_;
		}

		void Apply();

	protected:
		bool setEnvmap(
			const std::string& diffuseName,
			const std::string& specularName,
			const std::string& intergeFileName);
	};

	class SkyBox
	{
	private:
		StaticMeshRender   box_;
		D3D11GraphicsContext* context_;
	public:
		SkyBox() :context_(nullptr){}
		~SkyBox(){}
		bool Create()
		{
			False_Return_False( this->createRenderData() );
			context_ = D3D11GraphicsContext::GetSingletonPtr();
			return true;
		}

		void Render(ID3D11DeviceContext* context)
		{
			context_->DisableDepthTest();
			box_.Render(context);
			context_->EnableDepthTest();
		}
	private:
		bool createRenderData();
	}; //skybox


};//ul




#endif