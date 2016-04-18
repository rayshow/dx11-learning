#ifndef UL_RENDERABLE_HEADER__
#define UL_RENDERABLE_HEADER__

#include<vector>
#include<d3d11.h>
#include<D3DX11tex.h>

#include"../util/UlHelper.h"
#include"Material.h"


using namespace std;

namespace ul{

	const unsigned CONST_MAX_SHADER_RESOURCE_NUM = 16;

	enum EShaderResource_Type
	{
		eShaderResource_Albedo = 0,
		eShaderResource_Normal,
		eShaderResource_Specular,
		eShaderResource_Emit,
		eShaderResource_Irridiance,
		eShaderResource_SpecularLukup,
		eShaderResource_EnvCubemap,
		eShaderResource_IntergeLukup,
		eShaderResource_Tex1,
		eShaderResource_Tex2,
		eShaderResource_Tex3,
		eShaderResource_Cubemap1,
		eShaderResource_Cubemap2,
	};



	struct SRenderParameter
	{
		ID3D11VertexShader*         vsEnterPoint_;
		ID3D11PixelShader*          psEnterPoint_;
		ID3D11ShaderResourceView*   srvs_[CONST_MAX_SHADER_RESOURCE_NUM];
	};

	class SceneMgr;

	class SubBatch 
	{
	protected:
		ulUint							            texCount_;
		ulUint                                      indexOffset_;
		ulUint									    indexCount_;
		SRenderParameter*							refParameter_;
		SceneMgr*                                   refSceneMgr_;
		ID3D11Buffer*                               constBuffer_;
	public:
		SubBatch();
		virtual ~SubBatch() {}

		void ApplyMaterial(ID3D11DeviceContext* context);
		void Render(ID3D11DeviceContext* context);
		void SetConstBuffer(ID3D11Buffer* constBuffer)
		{
			this->constBuffer_ = constBuffer;
		}


		inline void SetRenderBatch(ulUint indexOffset, ulUint indexCount)
		{
			this->indexCount_ = indexCount;
			this->indexOffset_ = indexOffset;
		}

		inline void SetParameter(SRenderParameter *parameter)
		{
			this->refParameter_ = parameter;
		}
	};


	class BaseModel 
	{
	public:
		BaseModel() :vb_(nullptr), ib_(nullptr) {}

		virtual  ~BaseModel()
		{
			this->Release();
		}
	public:

		bool Create(ID3D11Device* pd3dDevice,
			SModelData& data);

		void SetShaderParameter(ID3D11DeviceContext* context){}

		void Render(ID3D11DeviceContext* context) ;

		void SetShader(ID3D11VertexShader* vs, ID3D11PixelShader* ps)
		{
			for (int i = 0; i < renderParameters_.size(); ++i)
			{
				SRenderParameter* pParameter = renderParameters_.at(i);
				pParameter->vsEnterPoint_ = vs;
				pParameter->psEnterPoint_ = ps;
			}
		}

		void SetShaderResource(ID3D11ShaderResourceView** views)
		{
			for (int i = 0; i < renderParameters_.size(); ++i)
			{
				SRenderParameter* pParameter = renderParameters_.at(i);
				memcpy(pParameter->srvs_, views, sizeof(ID3D11ShaderResourceView*)*CONST_MAX_SHADER_RESOURCE_NUM);
			}
		}

		void SetConstBuffer(ID3D11Buffer* buffer)
		{
			for (int i = 0; i < children_.size(); ++i)
			{
				children_.at(i).SetConstBuffer(buffer);
			}
		}
		void Release()
		{
			for (ulUint i = 0; i < renderParameters_.size(); ++i)
			{
				SRenderParameter *pParameter = renderParameters_[i];
				Safe_Delete(pParameter);
			}
		}
		
	protected:
		ID3D11Buffer*			       vb_;
		ID3D11Buffer*			       ib_;
		ID3D11InputLayout*			   vertexLayout_;
		ulUint					       stride_;
		ulUint						   offset_;
		ulUint						   childCount_;
		std::vector<SubBatch>		   children_;
		std::vector<SRenderParameter*> renderParameters_;
	};


	



}; //ul
#endif