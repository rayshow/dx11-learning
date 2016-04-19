#ifndef UL_MODEL_READER_HEADER__
#define UL_MODEL_READER_HEADER__


#include<string>
#include<vector>

#include<D3D11.h>
#include<Importer.hpp>
#include<scene.h>
#include<postprocess.h>

#include"../util/UlHelper.h"
#include"../render/VertexFormat.h"
#include"../render/Material.h"
#include"../render/MeshRender.h"

namespace ul
{
	struct STexturePosNamePair
	{
		ulUint		index;
		std::string name;
	};

	const STexturePosNamePair CONST_ALL_TEXTURE_POS_NAMES[CONST_MAX_TEXTURE_NUM] =
	{
		{ eShaderResource_Albedo,   "albedoMap"   },
		{ eShaderResource_Normal,   "normalMap"   },
		{ eShaderResource_Specular, "specularMap" },
		{ eShaderResource_Emit,		"emitMap" },
		{ eShaderResource_Emit + 1, "" },
		{ eShaderResource_Emit + 2, "" },
		{ eShaderResource_Emit + 3, "" },
		{ eShaderResource_Emit + 4, "" },
		{ eShaderResource_Emit + 5, "" },
		{ eShaderResource_Emit + 6, "" },
		{ eShaderResource_Emit + 7, "" },
		{ eShaderResource_Emit + 8, "" },
		{ eShaderResource_Emit + 9, "" },
		{ eShaderResource_Emit + 10, "" },
		{ eShaderResource_Emit + 11, "" },
	};
	

	class CommonModelLoader
	{
	public:
		CommonModelLoader()
		{
		}
		~CommonModelLoader()
		{
		}

		bool LoadFile(
			const std::string resourcePath,
			const std::string& fileName,
			SModelData& data);

	private:
		bool loadVerticeData(
			EVerticeType type,
			const aiMesh* mesh,
			void* buffer);

		bool loadIndiceData(
			aiMesh* mesh,
			ulUint* buffer);

		bool loadMaterial(
			const std::string& resourcePath,
			const std::string& materialFileName,
			std::vector<SMaterialData*>& materialGroup);
	};
};

#endif