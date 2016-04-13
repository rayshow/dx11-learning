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
		ID3D11ShaderResourceView*  environmentMaps_[3];
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
			const std::string& intergeFileName)
		{
			ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();

			Null_Return_False_With_Msg(
				(environmentMaps_[0] = mgr->CreateTextureFromFile(diffuseName)),
				"create diffuse env map from file:%s  error.",
				diffuseName.c_str()
			);

			Null_Return_False_With_Msg(
				(environmentMaps_[1] = mgr->CreateTextureFromFile(specularName)),
				"create specular env map from file:%s  error.",
				specularName.c_str()
				);

			Null_Return_False_With_Msg(
				(environmentMaps_[2] = mgr->CreateTextureFromFile(intergeFileName)),
				"create interge map from file:%s  error.",
				intergeFileName.c_str()
				);

			return true;
		}
	};


	class SkyBox : public BaseModel
	{
	private:
		Environmentable *environment_;

	public:
		SkyBox(){}
		~SkyBox(){}
		bool Create(
			ID3D11Device*    device,
			Environmentable* environment
			)
		{
			this->setEnvironment(environment);
			this->createRenderData(device);
			return true;
		}
	private:
		void setEnvironment(Environmentable* environment)
		{
			this->environment_ = environment;
		}
		void createRenderData(ID3D11Device* device)
		{
			SModelData data;
			ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();
			data.primtives_.stride_ = sizeof(SVertexXyzNuv);
			data.primtives_.indiceNum_ = 36;
			data.primtives_.type_ = eVertex_XYZNUV;
			data.primtives_.verticeNum_ = 8;
			data.primtives_.verticeBuffer_.resize(data.primtives_.verticeNum_*data.primtives_.stride_);
			data.primtives_.indices_.resize(36);

			SVertexXyzNuv* buffer = static_cast<SVertexXyzNuv*>((void*)&data.primtives_.verticeBuffer_[0]);
			VertexXyzNuv_Fill(buffer[0], -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			VertexXyzNuv_Fill(buffer[1], -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			VertexXyzNuv_Fill(buffer[2], 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			VertexXyzNuv_Fill(buffer[3], 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

			VertexXyzNuv_Fill(buffer[4], 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			VertexXyzNuv_Fill(buffer[5], 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			VertexXyzNuv_Fill(buffer[6], -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			VertexXyzNuv_Fill(buffer[7], -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

			data.primtives_.indices_ = {
				0, 1, 2,
				2, 3, 0,
				3, 2, 5,
				5, 4, 3,
				4, 5, 6,
				6, 7, 4,
				7, 6, 1,
				1, 0, 7,
				7, 0, 3,
				3, 4, 7,
				1, 6, 5,
				5, 2, 1 
			};

			SRenderGroupInfo *pGroup = new SRenderGroupInfo;
			pGroup->indexCount_ = data.primtives_.indiceNum_;
			pGroup->indexOffset_ = 0;
			data.groups_.push_back(pGroup);
			BaseModel::Create(device, data);
			ModelData_Free(data);

			SRenderParameter* pParameter = new SRenderParameter();
			pParameter->srvCount_ = 1;
			pParameter->srvs_ = new ID3D11ShaderResourceView*[1];
			pParameter->srvs_[0] = environment_->GetEnvironmentmaps()[1];
			renderParameters_.push_back(pParameter);

			D3D11_INPUT_ELEMENT_DESC desc[G_MAX_INPUT_ELEMENT_COUNT];
			ulUint count;
			ID3D11InputLayout* layout;
			GetInputDescByVerticeType(eVertex_XYZNUV, desc, &count);

			Null_Return_Void(
				(pParameter->vsEnterPoint_ = mgr->CreateVertexShaderAndInputLayout("skybox.hlsl", "VS_FillBuffer", "vs_5_0",
				desc, count, &pParameter->vertexLayout_))
			);
			pParameter->psEnterPoint_ = mgr->CreatePixelShader("skybox.hlsl", "PS_FillBuffer", "ps_5_0");


			this->children_[0].SetParameter(pParameter);
		}



	}; //skybox


};//ul




#endif