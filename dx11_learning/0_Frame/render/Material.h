#ifndef UL_MATERIAL_HEADER__
#define UL_MATERIAL_HEADER__

#include<vector>
#include<string>
#include"VertexFormat.h"
#include"../base/BaseDefine.h"

namespace ul
{
	const unsigned CONST_MAX_TEXTURE_NUM = 16;

	enum EMaterialParameterType
	{
		eMaterialParameter_Path = 0,
		eMaterialParameter_Float,
		eMaterialParameter_Int,
		eMaterialParameter_Vector,
		eMaterialParameter_Unkown
	};



	struct SMaterialParameter
	{
		std::string				 name;
		EMaterialParameterType   type;
		ulUint                   size;
		void*                    value;
	};




	struct SMaterialData{
		std::string					 identifer;
		std::string					 shaderFile;
		std::vector<SMaterialParameter*>  parameters;
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
		std::vector<ulUint>              indices_;
	};
	
	struct SModelData
	{
		std::string                                sourceFile_;
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