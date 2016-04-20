#ifndef UL_VERTEX_FORMAT_HEADER__
#define UL_VERTEX_FORMAT_HEADER__

#include<d3d11.h>
#include<D3DX11tex.h>
#include"../base/BaseDefine.h"

namespace ul
{
	enum EVerticeType{
		eVertex_XYZNUV,
		eVertex_XYZNUVTB,
		eVertex_XYZNUVTBIIIWW,
		eVertex_UNKNOW,
	};

	struct SVertexXyzNuv
	{
		ulFloat		pos_[3];
		ulFloat		normal_[3];
		ulFloat		uv_[2];
	};

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


	inline void Vertex_FillXyznuv(
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

	inline void Vertex_GetInputDescByType(EVerticeType type, D3D11_INPUT_ELEMENT_DESC* desc, ulUint* count)
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

	const ulUint CONST_MAX_INPUT_ELEMENT_COUNT = 7;

};



#endif