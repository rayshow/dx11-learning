#ifndef MODEL_READER__
#define MODEL_READER__

#include<string>
#include<vector>
#include<d3d11.h>
#include"tools.h"
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


struct VertexXyzNuv
{
	float		pos_[3];
	float		normal_[3];
	float		uv_[2];
	
	VertexXyzNuv(){}
	VertexXyzNuv(float *pos,
		float *normal,
		float *uv)
	{
		memcpy(pos_, pos, sizeof(float)* 3);
		memcpy(normal_, normal, sizeof(float)* 3);
		memcpy(uv_, uv, sizeof(float)* 3);
	}
};


struct VertexXyznuvtb : VertexXyzNuv
{
	float tangent_[3];
	float binormal_[3];

	VertexXyznuvtb(){}
	VertexXyznuvtb(
		float *pos,
		float *normal,
		float *uv,
		float *tangent,
		float *binormal) :VertexXyzNuv(pos, normal, uv)
	{
		memcpy(tangent_,  tangent, sizeof(float) * 3);
		memcpy(binormal_, binormal, sizeof(float) * 3);
	}
};

struct VertexXyznuvtbiiiww : VertexXyznuvtb
{
	float		       tangent_[3];
	float			   binormal_[3];
	unsigned char      iii_[4];
	unsigned char      ww_[4];

	VertexXyznuvtbiiiww(
		float *pos,
		float *normal,
		float *uv,
		float *tangent,
		float *binormal,
		unsigned char *iii,
		unsigned char *ww
		) :VertexXyznuvtb(pos, normal, uv, tangent, binormal)
	{
		
	}


};


enum eVerticeType{
	eVertex_XYZNUV,
	eVertex_XYZNUVTB,
	eVertex_XYZNUVTBIIIWW,
};

struct group_info9
{
	int startIndex;
	int primitives;
	int startVertex;
	int vertices;
};


struct MaterialData{
	std::string				 identifer;
	int						 texCount;
	std::vector<std::string> texturePath;
};


struct group_info11
{
	int indexOffset_;
	int indexCount_;
	MaterialData material_;
};

struct PrimitiveData
{	
	eVerticeType						   type_;
	int									   verticeNum_;
	int                                    stride_;
	std::vector<byte>                      verticeBuffer_;
	std::vector<unsigned short>            indices_;
};


struct ModelData
{
	PrimitiveData						   primData;
	std::vector<group_info11*>             groups_;
};


inline void ModelData_Free(ModelData& data)
{
	for (int i = 0; i < data.groups_.size(); ++i)
	{
		Safe_Delete(data.groups_[i]);
	}
}

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
		ModelData& data);

private:
	static bool LoadPrimitivesFile(
		ModelData& data,
		const std::string& primName);

	static bool LoadMaterialFile(
		ModelData& data,
		const std::string& matName);

	static bool ReadVerticesData(
		PrimitiveData& data,
		sec_info_t& info,
		FILE *fp);

	static bool ReadIndicesData(
		ModelData& data,
		sec_info_t& info,
		FILE *fp);

};





#endif