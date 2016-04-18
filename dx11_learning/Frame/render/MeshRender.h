#ifndef UL_RENDERABLE_HEADER__
#define UL_RENDERABLE_HEADER__

#include<vector>
#include<d3d11.h>
#include<D3DX11tex.h>

#include"../util/UlHelper.h"
#include"Material.h"


using namespace std;

namespace ul{

	enum EShaderResource_Type
	{
		eShaderResource_Albedo = 0,
		eShaderResource_Normal,
		eShaderResource_Specular,
		eShaderResource_Emit,
		eShaderResource_Irridiance,
		eShaderResource_SpecularLukup,
		eShaderResource_IntergeLukup,
		eShaderResource_EnvCubemap,
		eShaderResource_Tex1,
		eShaderResource_Tex2,
		eShaderResource_Tex3,
		eShaderResource_Cubemap1,
		eShaderResource_Cubemap2,
	};

	enum ESampler_Type
	{
		eSampler_Albedo = 0,
		eSampler_Normal,
		eSampler_Specular,
		eSampler_Emit,
		eSampler_Irridiance,
		eSampler_SpecularLukup,
		eSampler_EnvCubemap,
		eSampler_IntergeLukup,
		eSampler_Tex1,
		eSampler_Tex2,
		eSampler_Tex3,
		eSampler_Cubemap1,
		eSampler_Cubemap2,
	};



	struct SRenderParameter
	{
		ID3D11VertexShader*         vsEnterPoint_;
		ID3D11PixelShader*          psEnterPoint_;
		ID3D11ShaderResourceView*   srvs_[CONST_MAX_TEXTURE_NUM];
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

		void SetShaderResource(EShaderResource_Type target, ID3D11ShaderResourceView* view)
		{
			for (int i = 0; i < renderParameters_.size(); ++i)
			{
				SRenderParameter* pParameter = renderParameters_.at(i);
				pParameter->srvs_[target] = view;
			}
		}

		void SetShaderResources(EShaderResource_Type startPos, EShaderResource_Type end, ID3D11ShaderResourceView** view)
		{
			assert(end > startPos);
			assert(view != nullptr);

			for (int i = 0; i < renderParameters_.size(); ++i)
			{
				SRenderParameter* pParameter = renderParameters_.at(i);
				memcpy(&pParameter->srvs_[startPos], view, sizeof(ID3D11ShaderResourceView*)*(end - startPos + 1));
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