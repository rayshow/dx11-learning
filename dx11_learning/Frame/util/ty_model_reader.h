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


#define VERTEX_FORMAT_XYZNUVTB		    "xyznuvtb"
#define VERTEX_FORMAT_XYZNUV		    "xyznuv"
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


struct ModelVertex
{
	float		pos_[3];
	float		normal_[3];
	float		uv_[2];
	
	ModelVertex(){}
	ModelVertex(float *pos,
		float *normal,
		float *uv)
	{
		memcpy(pos_, pos, sizeof(float)* 3);
		memcpy(normal_, normal, sizeof(float)* 3);
		memcpy(uv_, uv, sizeof(float)* 3);
	}
};

struct VertexXyznuviiiwwtb : ModelVertex
{
	float      tangent_[3];
	float      binormal_[3];
	unsigned char      iii_[4];
	unsigned char       ww_[4];

	VertexXyznuviiiwwtb(){}
	VertexXyznuviiiwwtb(
		float *pos,
		float *normal,
		float *uv,
		float *tangent,
		float *binormal,
		float *iii,
		float *ww
		) :ModelVertex(pos, normal, uv)
	{
		//memcpy(tangent_, tangent, sizeof(float)* 3);
		//memcpy(binormal, binormal, sizeof(float)* 3);
		//memcpy(iii_, iii, sizeof(char)* 3);
		//memcpy(ww_, ww, sizeof(char)* 2);
	}

	static void print_data(VertexXyznuviiiwwtb&v, char* msg)
	{
	/*	sprintf(msg, "pos[%f, %f, %f], normal[%f, %f, %f], uv[%f, %f], tangent[%f, %f, %f], binormal[%f, %f, %f], iii[%d, %d, %d], ww[%d, %d]",
			v.pos_[X], v.pos_[Y], v.pos_[Z], v.normal_[X], v.normal_[Y], v.normal_[Z], 
			v.uv_[X], v.uv_[Y], v.tangent_[X], v.tangent_[Y], v.tangent_[Z], 
			v.binormal_[X], v.binormal_[Y], v.binormal_[Z], 
			v.iii_[X], v.iii_[Y], v.iii_[Z], v.ww_[X], v.ww_[Y]);*/
	}
};


enum eVerticeType{
	Vertex_XYZNUV,
	Vertex_XYZNUVIIIWWTB,
};

struct group_info9
{
	int startIndex;
	int primitives;
	int startVertex;
	int vertices;
};

struct group_info11
{
	int startIndex;
	int indiceCount;
	int startVertex;
	int vertexCount;
};

struct PrimitiveData{
	int verticeNum_;
	eVerticeType type_;
	byte                                   *verticesData_;
	std::vector<unsigned short>            indices;
	std::vector<group_info11*>             groups;
};


struct MaterialData{
	std::string identifer;
	std::string diffuseTexMap;
	std::string normalTexMap;
	std::string specularTexMap;
	std::string rgbTexMap;
	std::string renseTexMap;
	std::string diffuseTexMap2;
};


struct ModelData
{
	PrimitiveData primData;
	MaterialData  matData;
};


inline void ModelData_Free(ModelData* d)
{
	for (int i = 0; i < d->primData.groups.size(); ++i)
	{
		Safe_Delete(d->primData.groups[i]);
	}
	d->primData.groups.erase(d->primData.groups.begin(), d->primData.groups.end());

	Safe_Delete_Array(d->primData.verticesData_);
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
		const std::string& resDir,
		const std::string& modelName,
		bool noBelong,
		ModelData* data);

private:
	static bool LoadPrimitivesFile(
		PrimitiveData& data,
		const std::string& primName);

	static bool LoadMaterialFile(
		const std::string respath,
		MaterialData& data,
		const std::string& matName);

	static bool ReadVerticesData(
		PrimitiveData& data,
		sec_info_t& info,
		FILE *fp);

	static bool ReadIndicesData(
		PrimitiveData& data,
		sec_info_t& info,
		FILE *fp);

};





#endif