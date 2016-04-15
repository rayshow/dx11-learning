#ifndef UL_RENDERABLE_HEADER__
#define UL_RENDERABLE_HEADER__

#include<vector>
#include<d3d11.h>
#include<D3DX11tex.h>

#include"../util/UlHelper.h"
#include"Material.h"


using namespace std;

namespace ul{


	struct SRenderParameter
	{
		ulUint                      srvCount_;
		ID3D11InputLayout*			vertexLayout_;
		ID3D11VertexShader*         vsEnterPoint_;
		ID3D11PixelShader*          psEnterPoint_;
		ID3D11ShaderResourceView**  srvs_;
	};

	class Renderable
	{
	public:
		virtual ~Renderable(){}
		virtual void Render(ID3D11DeviceContext* context) = 0;

	}; 

	class SceneMgr;
	class SubBatch : public Renderable
	{
	protected:
		ulUint							            texCount_;
		ulUint                                      indexOffset_;
		ulUint									    indexCount_;
		SRenderParameter*							refParameter_;
		SceneMgr*                                   refSceneMgr_;
	public:
		SubBatch();
		virtual ~SubBatch() {}

		void ApplyMaterial(ID3D11DeviceContext* context);
		virtual void Render(ID3D11DeviceContext* context);

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


	class BaseModel : public Renderable
	{
	public:
		BaseModel() :vb_(nullptr), ib_(nullptr) {}

		virtual  ~BaseModel()
		{
			this->Release();
		}

		bool Create(ID3D11Device* pd3dDevice,
			SModelData& data);

		virtual void SetShaderParameter(ID3D11DeviceContext* context){}
		virtual void Render(ID3D11DeviceContext* context) override;

		void Release()
		{
			for (ulUint i = 0; i < renderParameters_.size(); ++i)
			{
				SRenderParameter *pParameter = renderParameters_[i];
				if (pParameter->srvCount_ > 0)
				{
					Safe_Delete_Array(pParameter->srvs_);
				}
				Safe_Delete(pParameter);
			}
		}
		
	protected:
		ID3D11Buffer*			       vb_;
		ID3D11Buffer*			       ib_;
		ulUint					       stride_;
		ulUint						   offset_;
		ulUint						   childCount_;
		std::vector<SubBatch>		   children_;
		std::vector<SRenderParameter*> renderParameters_;
	};


	



}; //ul
#endif