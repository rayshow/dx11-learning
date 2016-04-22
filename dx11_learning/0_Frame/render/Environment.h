#ifndef UL_SKYBOX_HEADER__
#define UL_SKYBOX_HEADER__

#include<string>
#include<d3d11.h>
#include<xnamath.h>

#include"MeshRender.h"

namespace ul
{
	class Environmentable
	{
	protected:
		ID3D11ShaderResourceView*  environmentMaps_[3];
		ID3D11SamplerState*        samplers_[3];
		ulFloat                    exposure_;
	public:
		Environmentable()
		{
			memset(environmentMaps_, 0, sizeof(ID3D11ShaderResourceView*) * 3);
			memset(samplers_, 0, sizeof(ID3D11SamplerState*) * 3);
		}

		bool SetEnvmaps(
			const std::string& diffuseName,
			const std::string& specularName,
			const std::string& intergeFileName)
		{
			False_Return_False(this->setEnvmap(diffuseName, specularName, intergeFileName));
			return true;
		}

		bool LoadFromFile(const string& fileName);

		virtual ~Environmentable(){}
	public:
		 ID3D11ShaderResourceView** GetEnvironmentmaps()
		 {
			 return environmentMaps_;
		 }

		 void ApplyEnvironment(ID3D11DeviceContext* context);

	protected:
		bool setEnvmap(
			const std::string& diffuseName,
			const std::string& specularName,
			const std::string& intergeFileName);

	};

	class SkyBox
	{
	public:
		struct SSkeyBox_Parameter
		{
			XMFLOAT4X4 rotateProject;
		};
	private:
		Environmentable    environment_;
		StaticMeshRender   model_;
		ID3D11Buffer*	   constBuffer_;
		SSkeyBox_Parameter parameter_;
	public:
		SkyBox():constBuffer_(nullptr) {}
		~SkyBox(){}
		bool Create(
			ID3D11Device*    device,
			std::string      fileName)
		{
			False_Return_False( environment_.LoadFromFile(fileName));
			False_Return_False( this->createRenderData(device) );
			return true;
		}

		void ApplySkyBox(ID3D11DeviceContext* context)
		{
			environment_.ApplyEnvironment(context);
		}
		
		void Render(ID3D11DeviceContext* context)
		{
			this->updateBuffer();
			model_.Render(context);
		}
	private:
		bool createRenderData(ID3D11Device* device);
		void updateBuffer();
	}; //skybox


};//ul




#endif