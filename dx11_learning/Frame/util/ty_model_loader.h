#ifndef MODEL_READER__
#define MODEL_READER__

#include<string>
#include<vector>
#include<d3d11.h>
#include"tools.h"
#include"renderable.h"

namespace ul
{

	#define MODEL_APPENDIX     ".model"
	#define PRIMITIVE_APPENDIX ".primitives"
	#define MATERIAL_APPENDIX  ".visual"
	#define VERTEX_FLAG        "vertices"
	#define INDICE_FLAG        "indices"

	#define VERTEX_FORMAT_XYZNUV		    "xyznuv"
	#define VERTEX_FORMAT_XYZNUVTB		    "xyznuvtb"
	#define VERTEX_FORMAT_XYZNUV2TB		    "xyznuv2tb"
	#define VERTEX_FORMAT_XYZNUVIIIWWTB		"xyznuviiiwwtb"
	#define VERTEX_FORMAT_XYZNUVIIIIWWWTB	"xyznuviiiiwwwtb"
	#define VERTEX_FORMAT_XYZNUVP2          "xyznuvp2"
	#define VERTEX_FORMAT_XYZNUVTBP2        "xyznuvtbp2"
	#define VERTEX_FORMAT_XYZNUVITB         "xyznuvitb"
	#define VERTEX_FORMAT_XYZNUVIIIWW       "xyznuviiiww"
	#define X 0
	#define Y 1
	#define Z 2
	#define W 3
	#define MAGIC_NUMBER       0x42a14e65


	struct sec_info_t
	{
		std::string   sec_file_name;
		unsigned long sec_size;
		unsigned long offset;
	};

	typedef std::vector<sec_info_t> sec_info_list;


	inline int PAD(unsigned long size, int padding)
	{
		return ((padding - size) % padding);
	}

	inline void UNPACKNORMAL(unsigned int packNromal, float* ret)
	{
		int z = int(packNromal) >> 22;
		int y = int(packNromal << 10) >> 21;
		int x = int(packNromal << 21) >> 21;

		ret[X] = (float)(x) / 1023.0f;
		ret[Y] = (float)(y) / 1023.0f;
		ret[Z] = (float)(z) / 511.0f;
	}




	class ModelReader{
	public:
		static bool Load(
			const std::string& textureDir,
			const std::string& modelName,
			bool loadAsSingleModel,
			bool useDDSTexture,
			SModelData& data);

	private:
		static bool LoadPrimitivesFile(
			SModelData& data,
			const std::string& primName);

		static bool LoadMaterialFile(
			SModelData& data,
			const std::string& matName);

		static bool ReadVerticesData(
			PrimitiveData& data,
			sec_info_t& info,
			FILE *fp);

		static bool ReadIndicesData(
			SModelData& data,
			sec_info_t& info,
			FILE *fp);

	};



}; //ul

#endif