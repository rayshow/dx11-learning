#ifndef UL_SKYBOX_HEADER__
#define UL_SKYBOX_HEADER__

#include<string>
#include<d3d11.h>
#include<xnamath.h>

#include"renderable.h"


namespace ul
{
	class Environmentable
	{
	protected:
		ID3D11ShaderResourceView*  environmentMaps_[3];
		ulFloat                    exposure_;
	public:
		Environmentable()
		{
			memset(environmentMaps_, 0, sizeof(ID3D11ShaderResourceView*) * 3);
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

	protected:
		bool setEnvmap(
			const std::string& diffuseName,
			const std::string& specularName,
			const std::string& intergeFileName);

	};

	class SkyBox : public BaseModel
	{
	public:
		struct SSkeyBox_Parameter
		{
			XMFLOAT4X4 viewProject;
		};
	private:
		Environmentable* environment_;
		ID3D11Buffer*    parameterBuffer_;
	public:
		SkyBox(){}
		~SkyBox(){}
		bool Create(
			ID3D11Device*    device,
			Environmentable* environment
			)
		{
			this->setEnvironment(environment);
			False_Return_False( this->createRenderData(device) );
			return true;
		}
	private:
		inline void setEnvironment(Environmentable* environment)
		{
			this->environment_ = environment;
		}
		
		bool createRenderData(ID3D11Device* device);

		virtual void SetShaderParameter(ID3D11DeviceContext* context) override;

	}; //skybox


};//ul




#endif