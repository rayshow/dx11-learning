#ifndef UL_RENDERABLE_HEADER__
#define UL_RENDERABLE_HEADER__


#include<vector>
#include<hash_map>
#include<d3d11.h>
#include<D3DX11tex.h>
#include<d3dx11effect.h>
#include<xnamath.h>

#include"../util/UlHelper.h"
#include"Material.h"



using namespace std;

namespace ul{

	class SceneMgr;
	class ResourceMgr;


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

	

	struct SRenderData
	{
		string                     effectName_;
		ID3DX11Effect*             effect_;
		vector<ID3DX11EffectPass*> passes_;
		ulUint                     passNum_;
		D3DX11_TECHNIQUE_DESC      desc_;
		ID3D11ShaderResourceView*  shaderSRVs_[CONST_MAX_TEXTURE_NUM];
		XMFLOAT4X4                 worldTransform_;
		XMFLOAT4X4                 worldViewProjectTransform_;
		
	};

	class StaticMeshPart
	{
	protected:
		ulUint              indexOffset_;
		ulUint				indexCount_;
		SRenderData*		refMaterial_;
	public:
		StaticMeshPart() 
		{};
		~StaticMeshPart() {}

		void Render(ID3D11DeviceContext* context);

		void SetMaterial(SRenderData* material)
		{
			refMaterial_ = material;
		}

		inline void SetRenderBatch(ulUint indexOffset, ulUint indexCount)
		{
			this->indexCount_ = indexCount;
			this->indexOffset_ = indexOffset;
		}
	};

	class StaticMeshRender
	{
	public:
		StaticMeshRender() :vb_(nullptr), ib_(nullptr), hasRenderPass_(false) {}
		~StaticMeshRender() 
		{
			for (ulUint i = 0; i < materials_.size(); ++i)
			{
				Safe_Delete(materials_[i]);
			}
			materials_.erase(materials_.begin(), materials_.end());
		}
		bool Create(ID3D11Device* pd3dDevice, const SModelData& data);
	public:
		
		void Render(ID3D11DeviceContext* context);

		template<typename XMFloat4x4>
		void SetParameter(string name, XMFloat4x4* value, ulUint number);
	
		void SetEffect(const string& fileName);

	protected:
		void setShaderFile(ResourceMgr* pResourceMgr, const string& shaderFileName, SRenderData* pRenderData);
		void setShaderParameter(const std::string effectName, ID3DX11Effect* pEffect,
			const SMaterialData* pMaterialData);
		void setInputLayout(ResourceMgr* pResourceMgr);
		void updateParameter();

	protected:
		ID3D11Buffer*			       vb_;
		ID3D11Buffer*			       ib_;
		EVerticeType                   vertexType_;
		ID3D11InputLayout*			   vertexLayout_;
		ulUint					       stride_;
		ulUint						   offset_;
		ulUint						   childCount_;
		std::vector<StaticMeshPart>	   children_;
		std::vector<SRenderData*>	   materials_;
		D3D11_INPUT_ELEMENT_DESC	   desc_[CONST_MAX_INPUT_ELEMENT_COUNT];
		ulUint                         descCount_;
		bool                           hasRenderPass_;
		string                         meshFileName_;
	};



}; //ul
#endif