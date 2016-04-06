#ifndef UL_RENDERABLE_HEADER__
#define UL_RENDERABLE_HEADER__

#include<d3d11.h>
#include<D3DX11tex.h>

#include<vector>
#include"tools.h"

namespace ul{
	using namespace std;

	struct SVertexXyzNuv
	{
		ulFloat		pos_[3];
		ulFloat		normal_[3];
		ulFloat		uv_[2];

		SVertexXyzNuv(){}
		SVertexXyzNuv(float *pos,
			float *normal,
			float *uv)
		{
			memcpy(pos_, pos, sizeof(float) * 3);
			memcpy(normal_, normal, sizeof(float) * 3);
			memcpy(uv_, uv, sizeof(float) * 3);
		}
	};


	struct SVertexXyznuvtb : SVertexXyzNuv
	{
		ulFloat tangent_[3];
		ulFloat binormal_[3];

		SVertexXyznuvtb(){}
		SVertexXyznuvtb(
			ulFloat *pos,
			ulFloat *normal,
			ulFloat *uv,
			ulFloat *tangent,
			ulFloat *binormal) :SVertexXyzNuv(pos, normal, uv)
		{
			memcpy(tangent_, tangent, sizeof(float) * 3);
			memcpy(binormal_, binormal, sizeof(float) * 3);
		}
	};

	struct SVertexXyznuvtbiiiww : SVertexXyznuvtb
	{
		ulFloat		       tangent_[3];
		ulFloat			   binormal_[3];
		ulUbyte			   iii_[4];
		ulUbyte            ww_[4];

		SVertexXyznuvtbiiiww(
			ulFloat *pos,
			ulFloat *normal,
			ulFloat *uv,
			ulFloat *tangent,
			ulFloat *binormal,
			ulUbyte *iii,
			ulUbyte *ww
			) :SVertexXyznuvtb(pos, normal, uv, tangent, binormal)
		{

		}
	};

	struct SMaterialData{
		std::string				 identifer;
		ulUint  				 texCount;
		std::vector<std::string> texturePath;
	};

	struct SGroupInfo
	{
		ulUint startIndex;
		ulUint primitives;
		ulUint startVertex;
		ulUint vertices;
	};

	struct SRenderGroupInfo
	{ 
		ulUint			   indexOffset_;
		ulUint			   indexCount_;
		ulUint			   materialID;
	};


	enum EVerticeType{
		eVertex_XYZNUV,
		eVertex_XYZNUVTB,
		eVertex_XYZNUVTBIIIWW,
		eVertex_UNKNOW,
	};

	struct PrimitiveData
	{
		EVerticeType					 type_;
		ulUint							 verticeNum_;
		ulUint                           stride_;
		ulUint                           indiceNum_;
		std::vector<ulUbyte>             verticeBuffer_;
		std::vector<ulUshort>            indices_;
	};


	struct SModelData
	{
		PrimitiveData							   primtives_;
		std::vector<SMaterialData*>                materials_;
		std::vector<SRenderGroupInfo*>             groups_;
	};

	inline void ModelData_Free(SModelData& data)
	{
		for (ulUint i = 0; i < data.groups_.size(); ++i)
		{
			Safe_Delete(data.groups_[i]);
			
		}
		for (ulUint i = 0; i < data.materials_.size(); ++i)
		{
			Safe_Delete(data.materials_[i]);
		}
	};


	const D3D11_INPUT_ELEMENT_DESC G_Layout_VertexXyznuv[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	const D3D11_INPUT_ELEMENT_DESC G_Layout_VertexXyznuvtb[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the shaders
	const D3D11_INPUT_ELEMENT_DESC G_Layout_VertexXyznuvtbiiiww[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "III", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WW", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};




	class Renderable
	{
	public:
		virtual void Render(ID3D11DeviceContext* context) = 0;
		virtual void Release() =0;
	};
 

	class SubBatch : public Renderable
	{
	protected:
		ulUint							            texCount_;
		ulUint                                      indexOffset_;
		ulUint									    indexCount_;
		ID3D11ShaderResourceView**                  refTextures_;
	public:
		SubBatch() :refTextures_(nullptr), texCount_(0), indexOffset_(0), indexCount_(0){}

		~SubBatch()
		{
			this->Release();
		}

		virtual void Render(ID3D11DeviceContext* context)
		{
			context->PSSetShaderResources(0, texCount_, refTextures_);
			context->DrawIndexed(indexCount_, indexOffset_, 0);
		}
		virtual void Release()
		{
			for (ulUint i = 0; i < texCount_; ++i)
			{
				Safe_Release(refTextures_[i]);
			}
			if (texCount_ >= 1)
			{
				Safe_Delete_Array(refTextures_);
			}
			int a = 0;

		}

		void SetRenderBatch(ulUint indexOffset, ulUint indexCount)
		{
			this->indexCount_ = indexCount;
			this->indexOffset_ = indexOffset;
		}

		void SetMaterial(ID3D11Device* dev, SMaterialData* pMaterial);
	};


	class BaseModel : public Renderable
	{
	public:
		BaseModel() :vb_(nullptr), ib_(nullptr) {}

	    ~BaseModel()
		{
			this->Release();
		}

		bool Create(ID3D11Device* pd3dDevice,
			SModelData& data);

		virtual void Render(ID3D11DeviceContext* mD3dImmediateContext) override;
		virtual void Release() override
		{
			Safe_Release(vb_);
			Safe_Release(ib_);			
		}
		
	protected:
		ID3D11Buffer*			    vb_;
		ID3D11Buffer*			    ib_;
		ulUint						stride_;
		ulUint						offset_;
		ulUint					    childCount_;
		std::vector<SubBatch>       children_;
		std::vector<SMaterialData*> materials_;
	};


	



}; //ul
#endif