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


namespace ul
{
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
			ulUshort* buffer);

		bool loadMaterial(
			const std::string& resourcePath,
			const std::string& materialFileName,
			std::vector<SMaterialData*>& materialGroup);
	};
};

#endif