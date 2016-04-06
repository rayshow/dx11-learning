#ifndef UL_SKYBOX_HEADER__
#define UL_SKYBOX_HEADER__

#include<string>
#include<d3d11.h>
#include"renderable.h"
#include"res_mgr.h"

namespace ul
{
	class Environmentable
	{
	protected:
		ID3D11ShaderResourceView*  specularEnvmap_;
		ID3D11ShaderResourceView*  diffuseEnvmap_;
	public:
		Environmentable(){}
		Environmentable(
			const std::string& diffuseName,
			const std::string& specularName):
			specularEnvmap_(nullptr),
			diffuseEnvmap_(nullptr)
		{
			this->setEnvmap(diffuseName, specularName);
		}

		virtual ~Environmentable(){}
	public:
	    ID3D11ShaderResourceView* GetSpecularEnvmap()
		{
			return specularEnvmap_;
		}
	 ID3D11ShaderResourceView* GetDiffuseEnvmap()
		{
			return diffuseEnvmap_;
		}
	private:
		void setEnvmap(
			const std::string& diffuseName,
			const std::string& specularName)
		{
			ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();
			if (Null((diffuseEnvmap_ = mgr->CreateTextureFromFile(diffuseName))))
			{
				Log_Err("create diffuse env map srv error.");
				assert(0);
			}
			if (Null((specularEnvmap_ = mgr->CreateTextureFromFile(specularName))))
			{
				Log_Err("create specular env map srv error.");
				assert(0);
			}
		}
	};

	class SkyBox : public BaseModel, public Environmentable
	{
	protected:
		ID3D11ShaderResourceView*  colorEnvmap_;
	public:
		SkyBox(
			const std::string& diffuseName,
			const std::string& specularName,
			const std::string& colorName) :
			Environmentable(diffuseName, specularName),
			colorEnvmap_(nullptr)
		{
			this->setColormap(colorName);
		}
		ID3D11ShaderResourceView* GetColorEnvmap()
		{
			return colorEnvmap_;
		}
	private:
		void setColormap(const std::string& colorName)
		{
			ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();
			if (Null((colorEnvmap_ = mgr->CreateTextureFromFile(colorName))))
			{
				Log_Err("create specular env map srv error.");
				assert(0);
			}
		}

		void createRenderData()
		{
			SModelData data;
			data.primtives_.stride_ = sizeof(SVertexXyzNuv);
			data.primtives_.indiceNum_ = 36;
			data.primtives_.type_ = eVertex_XYZNUV;
			data.primtives_.verticeNum_ = 24;


		}
	};


};




#endif