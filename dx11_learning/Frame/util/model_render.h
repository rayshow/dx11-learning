#ifndef UL_MODEL_READER_HEADER__
#define UL_MODEL_READER_HEADER__

#include<D3D11.h>
#include<string>
#include<vector>
#include<Importer.hpp>
#include<scene.h>
#include<postprocess.h>
#include"tools.h"


namespace ul
{
	enum eVertexType
	{
		Vertex_Xyznuv,
	};

	struct VertexXyznuv
	{
		ulFloat pos_[3];
		ulFloat normal_[3];
		ulFloat coord_[2];
	};

	struct VertexXyznuvtb
	{
		ulFloat pos_[3];
		ulFloat normal_[3];
		ulFloat coord_[2];
		ulFloat tangent_[3];
		ulFloat binormal_[3];
	};

	const D3D11_INPUT_ELEMENT_DESC VertexXyznuv_Layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	struct SubMesh
	{
		ID3D11Buffer*	vb_;
		ID3D11Buffer*	ib_;
		ulUint          stride_;
		ulUint          offset_;
		ulUint          indexCount_;
		ulUint          materialIndex_;
	};

	class ModelRender
	{
	public:
		ModelRender()
		{
			
		}
		~ModelRender()
		{
			MeshIter begin = submeshs_.begin();
			MeshIter end = submeshs_.end();
			for (MeshIter i = begin; i != end; ++i)
			{
				Safe_Delete(*i);
			}
			submeshs_.erase(begin, end);
		}

		bool LoadMesh(const std::string& fileName);
		void Render(ID3D11DeviceContext* context);
	private:
		bool initMesh(const aiMesh* mesh);
		bool initMaterial(const aiMaterial* material);
	private:
		std::vector<SubMesh*> submeshs_;
		std::vector<ID3D11ShaderResourceView*> textures_;
		ulUint  sumMeshCount_;
		typedef std::vector<SubMesh*>::iterator MeshIter;
		typedef std::vector<ID3D11ShaderResourceView*> TextureIter;
	
	};
};

#endif