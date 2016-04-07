
#include<exception>
#include<cstdio>
#include<sstream>
#include<algorithm>
#include<cassert>

#include<rapidxml.hpp>
#include<rapidxml_utils.hpp>
#include"ty_model_loader.h"

#pragma warning (disable : 4996)


using namespace rapidxml;
using namespace ul;

bool ModelReader::Load(
	const std::string& textureDir,
	const std::string& modelName,
	bool loadAsSingleModel,
	bool useDDSTexture,
	SModelData& data)
{
	std::string dir = textureDir;
	std::string localDir="";
	//preprocess
	if (modelName.length() == 0)
	{
		return false;
	}
	int off = (std::max)((int)textureDir.find_last_of("/"), (int)textureDir.find_last_of("\\")) + 1;
	if (off != dir.length())
	{
		dir += "/";
	}
	off = (std::max)((int)modelName.find_last_of("/"), (int)modelName.find_last_of("\\")) + 1;
	if (off != 0)
	{
		localDir = modelName.substr(0, off);
	}
	
	std::string fullName = loadAsSingleModel ? modelName : dir + modelName;// +MODEL_APPENDIX;

	bool ret = false;
	std::string primitiveName = "";
	std::string materialName = "";
	std::string onlyName = "";
	std::string visualPath = "";
	std::stringstream strBuffer;
	//load from .model file


	try{
		file<> fl(fullName.c_str());
		xml_document<> doc;
		doc.parse<0>(fl.data());

		//root
		xml_node<> *root = doc.first_node();
		if (root)
		{
			//nodelessVisual
			xml_node<> *nodelessVisual = nullptr;
			if( (nodelessVisual = root->first_node("nodelessVisual")) == nullptr)
				nodelessVisual = root->first_node("nodefullVisual");

			if (nodelessVisual)
			{
				//.visual file name
				strBuffer << nodelessVisual->value();
				strBuffer >> visualPath;
				
				//full path under game res dir
				if (!loadAsSingleModel)
				{
					primitiveName = dir + visualPath + PRIMITIVE_APPENDIX;
					materialName  = dir + visualPath + MATERIAL_APPENDIX;
				}
				else{
					int seperateOff = visualPath.find_last_of('/')+1;
					onlyName = visualPath.substr(seperateOff, visualPath.length() - seperateOff);
					primitiveName = localDir + onlyName + PRIMITIVE_APPENDIX;
					materialName = localDir + onlyName + MATERIAL_APPENDIX;
				}
				
				//read primitive data
				ret = LoadPrimitivesFile(data, primitiveName);

				//read material data
				ret &= LoadMaterialFile( data, materialName);

				string texPath;

				//remove dirPath from read and add textureDir
				for (unsigned int i = 0; i < data.groups_.size(); ++i)
				{
					SMaterialData* materialData = data.materials_[i];

					for (unsigned int j = 0; j < materialData->texCount; ++j)
					{
						texPath = materialData->texturePath[j];

						if (useDDSTexture)
						{
							int lastDot = texPath.find_last_of('.');
							texPath = texPath.substr(0, lastDot) + ".dds";
						}

						if (loadAsSingleModel)
						{
							int lastSeperate = texPath.find_last_of('/');
							texPath = dir + texPath.substr(lastSeperate + 1);
						}
						materialData->texturePath[j] = texPath;
					}
				}

				if ( !ret )
				{
					return false;
				}
			}
		}
	}
	catch (std::exception ex){
		return false;
	}

	return true;
}


bool ModelReader::LoadPrimitivesFile(
	SModelData& data,
	const std::string& primName)
{
	FILE *fp;
	fp = fopen(primName.c_str(), "rb");
	if (!fp)
	{
		assert(0);
		return false;
	}

	//valid magic number
	unsigned long magic_number = 0;
	fread(&magic_number, 4, 1, fp);

	if (magic_number != MAGIC_NUMBER)
	{
		
		fclose(fp);
		assert(0);
		return false;
	}
	
	//read index size
	unsigned long index_size = 0;
	fseek(fp, -4, SEEK_END);
	fread(&index_size, 4, 1, fp);
	if (index_size % 4 != 0)
	{
		fclose(fp);
		assert(0);
		return false;
	}


	fseek(fp, -4 - index_size, SEEK_END);
	unsigned long readsize = 0;
	unsigned long calc_size = 0;
	calc_size += sizeof(magic_number);
	
	//read sec blocks
	sec_info_list sec_list;

	unsigned long index = 0;

	char sec_file_name[MAX_PATH];
	while (readsize < index_size)
	{
		unsigned long nums[6];
		fread(nums, 4, 6, fp);
		readsize += 4 * 6;
		unsigned long sec_size = nums[0];
		unsigned long secname_size = nums[5];
		std::string scename;
		memset(sec_file_name, 0, MAX_PATH);
		fread(sec_file_name, 1, secname_size, fp);
		readsize += secname_size;

		int pad_size = PAD(secname_size, 4);
		if (pad_size)
		{
			int pad_size_content;
			fread(&pad_size_content, 1, pad_size, fp);
			readsize += pad_size;
		}

		sec_info_t new_sec;
		new_sec.sec_file_name = sec_file_name;
		new_sec.sec_size = sec_size;
		new_sec.offset = calc_size;
		sec_list.push_back(new_sec);

		calc_size += sec_size;
		calc_size += PAD(sec_size, 4);
		index++;
	}
	calc_size += index_size;
	calc_size += 4;

	fseek(fp, 0, SEEK_END);
	unsigned long fsize = ftell(fp);

	if (fsize != calc_size)
	{
		assert(0);
	}

	//read vertex and indice data
	for (size_t i = 0; i < sec_list.size(); i++)
	{
		if (sec_list[i].sec_file_name.find(VERTEX_FLAG) != std::string::npos ||
			sec_list[i].sec_file_name == VERTEX_FLAG)
		{
			if (!ReadVerticesData(data.primtives_, sec_list[i], fp))
			{
				fclose(fp);
				assert(0);
				return false;
			}
		}
		else if (sec_list[i].sec_file_name.find(INDICE_FLAG) != std::string::npos ||
			sec_list[i].sec_file_name == INDICE_FLAG)
		{
			if (!ReadIndicesData(data, sec_list[i], fp))
			{
				fclose(fp);
				assert(0);
				return false;
			}
		}
	}
	
	fclose(fp);
	
	return true;
}


bool ModelReader::ReadVerticesData(
	PrimitiveData& data,
	sec_info_t& sec,
	FILE *fp)
{

	char vertexformat[128];
	int  vertex_number = 0;
	memset(vertexformat, 0, 128);
	fseek(fp, sec.offset, SEEK_SET);
	fread(&vertexformat, 1, 64, fp);
	fread(&vertex_number, sizeof(int), 1, fp);

	std::string vertexFormatClean = "";
	for (size_t i = 0; i<strlen(vertexformat); i++)
	{
		if ((vertexformat[i] >= 'a' && vertexformat[i] <= 'z') || (vertexformat[i] == '2'))
		{
			vertexFormatClean += vertexformat[i];
		}
		else
		{
			break;
		}
	}

	bool b_uv2 = false;
	if (vertexFormatClean.find("2uv") != std::string::npos || vertexFormatClean.find("uv2") != std::string::npos)
	{
		b_uv2 = true;
	}

	unsigned int uint_normal, uint_binormal, uint_tangent;

	if (VERTEX_FORMAT_XYZNUV == vertexFormatClean)
	{
		assert(0);
		for (int i = 0; i < vertex_number; i++)
		{
			/*ModelVertex new_vertex;
			fread(&new_vertex.pos_, sizeof(float)*3, 1, fp);
			fread(&uint_normal, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_normal, new_vertex.normal_);
			fread(&new_vertex.uv_, sizeof(float)*2, 1, fp);

			data.vertices.push_back(new_vertex);*/
		}
	}
	else if (VERTEX_FORMAT_XYZNUVTB == vertexFormatClean)
	{
		data.verticeNum_ = vertex_number;
		data.type_ = eVertex_XYZNUVTBIIIWW;
		data.stride_ = sizeof(SVertexXyznuvtbiiiww);
		data.verticeBuffer_.resize(vertex_number*data.stride_);
		SVertexXyznuvtbiiiww* buffer = static_cast<SVertexXyznuvtbiiiww*>((void*)&data.verticeBuffer_[0]);

		for (int i = 0; i < vertex_number; i++)
		{
			SVertexXyznuvtbiiiww *pVertex = &buffer[i];
			memset(pVertex, 0, sizeof(SVertexXyznuvtbiiiww));

			fread(&pVertex->pos_, sizeof(float) * 3, 1, fp);
			fread(&uint_normal, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_normal, pVertex->normal_);
			fread(&pVertex->uv_, sizeof(float) * 2, 1, fp);
			fread(&uint_tangent, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_tangent, pVertex->tangent_);
			fread(&uint_binormal, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_binormal, pVertex->binormal_);


		}
	}
	else if (VERTEX_FORMAT_XYZNUV2TB == vertexFormatClean)
	{
		assert(0);
		for (int i = 0; i < vertex_number; i++)
		{
			/*ModelVertex new_vertex;
			fread(&new_vertex.pos_, sizeof(float)*3, 1, fp);
			fread(&uint_normal, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_normal, new_vertex.normal_);
			fread(&new_vertex.uv_, sizeof(float)*2, 1, fp);
			fread(uv2, sizeof(float)*2, 1, fp);
			fread(&uint_tangent, sizeof(unsigned int), 1, fp);
			fread(&uint_bitangent, sizeof(unsigned int), 1, fp);

			data.vertices.push_back(new_vertex);*/
		}
	}
	else if (VERTEX_FORMAT_XYZNUVIIIWWTB == vertexFormatClean)
	{
		data.verticeNum_ = vertex_number;
		data.type_ = eVertex_XYZNUVTBIIIWW;
		data.stride_ = sizeof(SVertexXyznuvtbiiiww);
		data.verticeBuffer_.resize(vertex_number*data.stride_);
		SVertexXyznuvtbiiiww* buffer = static_cast<SVertexXyznuvtbiiiww*>((void*)&data.verticeBuffer_[0]);
		
		for (int i = 0; i < vertex_number; i++)
		{
			SVertexXyznuvtbiiiww *pVertex = &buffer[i];
			memset(pVertex, 0, sizeof(SVertexXyznuvtbiiiww));

			fread(&pVertex->pos_, sizeof(float) * 3, 1, fp);
			fread(&uint_normal, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_normal, pVertex->normal_);
			fread(&pVertex->uv_, sizeof(float) * 2, 1, fp);

			fread(pVertex->iii_, sizeof(char), 3, fp);            //iii
			fread(pVertex->ww_, sizeof(char), 2, fp);             //ww

			fread(&uint_tangent, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_tangent, pVertex->tangent_);
			fread(&uint_binormal, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_binormal, pVertex->binormal_);
		}
	}

	else if (VERTEX_FORMAT_XYZNUVP2 == vertexFormatClean)
	{
		assert(0);
		//return false;
		for (int i = 0; i < vertex_number; i++)
		{
			/*ModelVertex new_vertex;
			fread(&new_vertex.pos_, sizeof(float)*3, 1, fp);
			fread(&uint_normal, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_normal, new_vertex.normal_);
			fread(&new_vertex.uv_, sizeof(float)*2, 1, fp);

			fread(iw, sizeof(unsigned char), 16, fp);

			data.vertices.push_back(new_vertex);*/
		}
	}
	else if (VERTEX_FORMAT_XYZNUVTBP2 == vertexFormatClean)
	{
		for (int i = 0; i < vertex_number; i++)
		{
			assert(0);
			/*ModelVertex new_vertex;
			fread(&new_vertex.pos_, sizeof(float)*3, 1, fp);
			fread(&uint_normal, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_normal, new_vertex.normal_);
			fread(&new_vertex.uv_, sizeof(float)*2, 1, fp);

			fread(&uint_tangent, sizeof(unsigned int), 1, fp);
			fread(&uint_bitangent, sizeof(unsigned int), 1, fp);
			fread(iw, sizeof(unsigned char), 16, fp);
			data.vertices.push_back(new_vertex);*/
		}
	}
	else if (VERTEX_FORMAT_XYZNUVITB == vertexFormatClean)
	{
		for (int i = 0; i < vertex_number; i++)
		{
			assert(0);
			/*ModelVertex new_vertex;
			fread(&new_vertex.pos_, sizeof(float)*3, 1, fp);
			fread(&uint_normal, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_normal, new_vertex.normal_);
			fread(&new_vertex.uv_, sizeof(float)*2, 1, fp);

			fread(iw, sizeof(unsigned char), 4, fp);
			fread(&uint_tangent, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_tangent, normal);
			fread(&uint_bitangent, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_bitangent, normal);

			data.vertices.push_back(new_vertex);*/
		}
	}
	else if (VERTEX_FORMAT_XYZNUVIIIWW == vertexFormatClean)
	{
		for (int i = 0; i < vertex_number; i++)
		{
			assert(0);
			/*ModelVertex new_vertex;
			fread(&new_vertex.pos_, sizeof(float)*3, 1, fp);
			fread(&uint_normal, sizeof(unsigned int), 1, fp);
			UNPACKNORMAL(uint_normal, new_vertex.normal_);
			fread(&new_vertex.uv_, sizeof(float)*2, 1, fp);
			fread(iw, 1, 5, fp);

			data.vertices.push_back(new_vertex);*/
		}
	}
	else{
		assert(0);
		return false;
	}



	return true;
}

bool ModelReader::ReadIndicesData(
	SModelData& data,
	sec_info_t& sec,
	FILE *fp)
{
	char indexFormat[128];
	int  index_number = 0;
	int  group_number = 0;
	memset(indexFormat, 0, 128);
	fseek(fp, sec.offset, SEEK_SET);
	fread(&indexFormat, 1, 64, fp);
	fread(&index_number, sizeof(int), 1, fp);
	fread(&group_number, sizeof(int), 1, fp);

	unsigned short index;
	for (int i = 0; i < index_number; i++)
	{
		fread(&index, sizeof(unsigned short), 1, fp);
		data.primtives_.indices_.push_back(index);
	}

	data.groups_.resize(group_number);
	data.materials_.resize(group_number);

	for(int i=0; i<group_number; ++i)
	{
		SGroupInfo info;
		fread(&info, sizeof(SGroupInfo), 1, fp);
		data.groups_[i] = new SRenderGroupInfo;
		data.materials_[i] = new SMaterialData;
		data.groups_[i]->materialID = i;
		data.groups_[i]->indexCount_ = info.primitives * 3;
		data.groups_[i]->indexOffset_ = info.startIndex;
	}

	if (data.primtives_.indices_.size() <= 0 || data.groups_.size()<0)
	{
		assert(0);
		return false;
	}
	return true;
}

bool ModelReader::LoadMaterialFile(
	SModelData& modelData,
	const std::string& matName)
{
	std::string  val;
	std::stringstream ss;
	float min[3], max[3];

	//load from .model file
	try{
		file<> fl(matName.c_str());
		xml_document<> doc;
		doc.parse<0>(fl.data());
		
		//root
		xml_node<> *root = doc.first_node();
		
		if (root)
		{
			//renderSet
			xml_node<> *boundingBox = root->first_node("boundingBox");
			
			//bounding box
			if (!boundingBox)
			{
				assert(0);
				return false;
			}
			else{
				xml_node<> *minNode = boundingBox->first_node("min");
				xml_node<> *maxNode = boundingBox->first_node("max");
				if (minNode)
				{
					ss << minNode->value();
					ss >> min[X] >> min[Y] >> min[Z];
				}
				else{
					assert(0);
					return false;
				}
				ss.str("");
				ss.clear();
				if (maxNode)
				{
					ss << maxNode->value();
					ss >> max[X] >> max[Y] >> max[Z];
				}
				else{
					assert(0);
					return false;
				}

				//data.aabb.init(min, max);
			}
			
			xml_node<> *renderSet = root->first_node("renderSet");
			if (!renderSet)
			{
				
				assert(0);
				return false;
			}

			//geometry
			xml_node<> *geometry = renderSet->first_node("geometry");
			if (!geometry)
			{
				assert(0);
				return false;
			}

			//primitiveGroup
			xml_node<> *primitiveGroup = geometry->first_node("primitiveGroup");
			if (!primitiveGroup)
			{
				assert(0);
				return false;
			}
			int index = 0;
			for (; primitiveGroup != nullptr; primitiveGroup = primitiveGroup->next_sibling())
			{
				SMaterialData* pMaterial = modelData.materials_[index++];
				std::string groupVal = primitiveGroup->value();

				//material
				xml_node<> *material = primitiveGroup->first_node("material");

				if (material)
				{
					//identifer
					xml_node<> *identifier = material->first_node("identifier");
					char *v = material->value();
					if (identifier)
					{
						pMaterial->identifer = identifier->value();
					}

					//property
					xml_node<> *prop = material->first_node("property");
					for (; prop != nullptr; prop = prop->next_sibling())
					{
						xml_node<>* texture = prop->first_node("Texture");

						if (!texture)
						{
							continue;
						}
						string texturePath = texture->value();
						pMaterial->texturePath.push_back(texturePath);
			
					}//for_property

					pMaterial->texCount = pMaterial->texturePath.size();

				}//if_material
			}//for_primitive_group
		}// if_root
	}//try
	catch (std::exception ex){
		assert(0);
		return false;
	}

	return true;
}