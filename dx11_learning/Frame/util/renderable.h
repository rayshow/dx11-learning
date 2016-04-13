#ifndef UL_RENDERABLE_HEADER__
#define UL_RENDERABLE_HEADER__

#include<d3d11.h>
#include<D3DX11tex.h>

#include<vector>
#include"tools.h"
//#include"../scene_mgr.h"

namespace ul{
	using namespace std;

	struct SVertexXyzNuv
	{
		ulFloat		pos_[3];
		ulFloat		normal_[3];
		ulFloat		uv_[2];
	};

	inline void VertexXyzNuv_Fill(
		SVertexXyzNuv& vertex,
		float x, float y, float z,
		float nx, float ny, float nz, float u, float v)
	{
		vertex.pos_[0] = x;
		vertex.pos_[1] = y;
		vertex.pos_[2] = z;
		vertex.normal_[0] = nx;
		vertex.normal_[1] = ny;
		vertex.normal_[2] = nz;
		vertex.uv_[0] = u;
		vertex.uv_[1] = v;
	}

	struct SVertexXyznuvtb : SVertexXyzNuv
	{
		ulFloat tangent_[3];
		ulFloat binormal_[3];
	};

	struct SVertexXyznuvtbiiiww : SVertexXyznuvtb
	{
		ulFloat		       tangent_[3];
		ulFloat			   binormal_[3];
		ulUbyte			   iii_[4];
		ulUbyte            ww_[4];
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

	const ulUint G_MAX_INPUT_ELEMENT_COUNT = 7;

	struct SMaterialData{
		std::string				 identifer;
		ulUint  				 texCount;
		std::string              shaderFile;
		std::string              vsEnterPoint;
		std::string              psEnterPoint;
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

	inline void GetInputDescByVerticeType(EVerticeType type, D3D11_INPUT_ELEMENT_DESC* desc, ulUint* count)
	{
		switch (type)
		{
		case eVertex_XYZNUV:
			memcpy(desc, G_Layout_VertexXyznuv, sizeof(G_Layout_VertexXyznuv));
			*count = 3;
			return;
		case eVertex_XYZNUVTB:
			memcpy(desc, G_Layout_VertexXyznuvtb, sizeof(G_Layout_VertexXyznuvtb));
			*count = 5;
			return;
		case eVertex_XYZNUVTBIIIWW:
			memcpy(desc, G_Layout_VertexXyznuvtbiiiww, sizeof(G_Layout_VertexXyznuvtbiiiww));
			*count = 7;
			return;
		}
	}

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

		virtual void Render(ID3D11DeviceContext* mD3dImmediateContext) override;

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