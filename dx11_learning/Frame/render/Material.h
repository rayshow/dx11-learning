#ifndef UL_MATERIAL_HEADER__
#define UL_MATERIAL_HEADER__

#include<vector>
#include"VertexFormat.h"
#include"../base/BaseDefine.h"

namespace ul
{
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

};

#endif