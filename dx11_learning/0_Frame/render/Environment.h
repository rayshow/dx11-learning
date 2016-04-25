#ifndef UL_SKYBOX_HEADER__
#define UL_SKYBOX_HEADER__

#include<string>
#include<d3d11.h>
#include<xnamath.h>

#include"MeshRender.h"

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
	public:
		SkyBox(){}
		~SkyBox(){}
		bool Create()
		{
			False_Return_False( this->createRenderData() );
			return true;
		}

		void Render(ID3D11DeviceContext* context)
		{
			box_.Render(context);
		}
	private:
		bool createRenderData();
	}; //skybox


};//ul




#endif