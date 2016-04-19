#include<rapidxml.hpp>
#include<rapidxml_utils.hpp>
#include<sstream>
#include<exception>

#include"CommonModelLoader.h"

using namespace ul;
using namespace rapidxml;
using namespace std;

bool CommonModelLoader::LoadFile(const std::string resourcePath, const std::string& fileName
	, SModelData& data)
{
	const std::string meshFileName = resourcePath + fileName;

	Assimp::Importer import;
	const aiScene* pScene = import.ReadFile(meshFileName.c_str(),
		aiProcess_Triangulate | aiProcess_SortByPType| aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

	Null_Return_False_With_Msg( pScene, "read model file %s error: %s.",
		meshFileName.c_str(), import.GetErrorString());

	if ( Zero(pScene->mNumMeshes) || Null(pScene->mMeshes[0]) )
	{
		Log_Err("read model file %s error", meshFileName.c_str());
		return false;
	}

	if ( pScene->mMeshes[0]->mFaces[0].mNumIndices!=3)
	{
		Log_Err("frame only support triangle primitive! from file: %s", meshFileName.c_str());
		return false;
	}


	//vertex type
	data.sourceFile_ = meshFileName;
	data.primtives_.type_ = eVertex_UNKNOW;
	if (pScene->mMeshes[0]->HasPositions() && pScene->mMeshes[0]->HasNormals() &&
		pScene->mMeshes[0]->HasTextureCoords(0))
	{
		data.primtives_.type_ = eVertex_XYZNUV;
		data.primtives_.stride_ = sizeof(SVertexXyzNuv);
	}
	
	if ( data.primtives_.type_ == eVertex_XYZNUV &&
		pScene->mMeshes[0]->HasTangentsAndBitangents())
	{
		data.primtives_.type_   = eVertex_XYZNUVTB;
		data.primtives_.stride_ = sizeof(SVertexXyznuvtb);
	}
	if (data.primtives_.type_ == eVertex_UNKNOW)
	{
		Log_Err("model file %s format not support", fileName.c_str());
		return false;
	}

	data.groups_.reserve(pScene->mNumMeshes);
	data.materials_.reserve(pScene->mNumMaterials);
	
	data.primtives_.verticeNum_ = 0;
	data.primtives_.indiceNum_ = 0;
	for (ulUint i = 0; i < pScene->mNumMeshes; ++i)
	{
		data.primtives_.verticeNum_ += pScene->mMeshes[i]->mNumVertices;
		data.primtives_.indiceNum_ += pScene->mMeshes[i]->mNumFaces * 3;
	}
	data.primtives_.verticeBuffer_.resize(data.primtives_.verticeNum_*data.primtives_.stride_);
	data.primtives_.indices_.resize(data.primtives_.indiceNum_);

	//load group
	ulUint bufferOffset = 0;
	ulUint indexOffset = 0;
	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
	{
		aiMesh* mesh = pScene->mMeshes[i];

		//render group info
		SRenderGroupInfo *pGroup = new SRenderGroupInfo();
		pGroup->materialID = i;
		pGroup->indexCount_ = mesh->mNumFaces * 3;
		pGroup->indexOffset_ = indexOffset;

		//data
		this->loadVerticeData(data.primtives_.type_, mesh, &data.primtives_.verticeBuffer_[bufferOffset]);
		this->loadIndiceData(mesh, &data.primtives_.indices_[indexOffset]);
		data.groups_.push_back(pGroup);

		//offset
		bufferOffset += mesh->mNumVertices * data.primtives_.stride_;
		indexOffset += mesh->mNumFaces * 3;
	}


	//load material
	std::string::size_type off = meshFileName.find_last_of(".");
	if (off == -1)
	{
		Log_Err("file name %s error", meshFileName.c_str());
		return false;
	}
	std::string materialName = meshFileName.substr(0, off) + ".material";
	this->loadMaterial(resourcePath, materialName, data.materials_ );

	return true;
}


bool CommonModelLoader::loadVerticeData(EVerticeType type, const aiMesh* mesh, void* buffer)
{
	switch (type)
	{
		case eVertex_XYZNUV:
		{
			SVertexXyzNuv *batch = static_cast<SVertexXyzNuv*>(buffer);
			for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
			{
				SVertexXyzNuv* pVertex = &batch[i];
				memcpy(pVertex->pos_, &mesh->mVertices[i], sizeof(float) * 3);
				memcpy(pVertex->uv_,  &mesh->mTextureCoords[0][i], sizeof(float) * 2);
				memcpy(pVertex->normal_, &mesh->mNormals[i], sizeof(float) * 3);
			}
			break;
		}
		case eVertex_XYZNUVTB:
		{
			SVertexXyznuvtb *batch = static_cast<SVertexXyznuvtb*>(buffer);
			for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
			{
				SVertexXyznuvtb* pVertex = &batch[i];
				memcpy(pVertex->pos_, &mesh->mVertices[i], sizeof(float) * 3);
				memcpy(pVertex->uv_, &mesh->mTextureCoords[0][i], sizeof(float) * 2);
				memcpy(pVertex->normal_, &mesh->mNormals[i], sizeof(float) * 3);
				memcpy(pVertex->tangent_, &mesh->mTangents[i], sizeof(float) * 3);
				memcpy(pVertex->binormal_, &mesh->mBitangents[i], sizeof(float) * 3);
			}
			break;
		}

		default:
			Log_Err("format not support!");
			return false;
	}
	return true;
}


bool CommonModelLoader::loadIndiceData(aiMesh* mesh, ulUint* buffer)
{
	for (ulUint i = 0; i < mesh->mNumFaces; ++i)
	{
		assert(mesh->mFaces[i].mNumIndices == 3);
		buffer[3 * i]   = mesh->mFaces[i].mIndices[0];
		buffer[3 * i+1] = mesh->mFaces[i].mIndices[1];
		buffer[3 * i+2] = mesh->mFaces[i].mIndices[2];
	}
	return true;
}

bool CommonModelLoader::loadMaterial(
	const std::string& resourcePath,
	const std::string& materialFileName,
	std::vector<SMaterialData*>& materialGroup)
{

	std::stringstream buffer;
	std::string mapName;
	int index = 0;
	

	try{
		file<> fl(materialFileName.c_str());
		xml_document<> doc;
		doc.parse<0>(fl.data());

		//nodelessVisual
		xml_node<> *materials = nullptr;
		if ((materials = doc.first_node("materials")) == nullptr)
		{
			Log_Err("can't find material data in %s", materialFileName.c_str());
			return false;
		}

		xml_node<> *material = materials->first_node("material");
		for (; material != nullptr; material = material->next_sibling("material"))
		{
			SMaterialData* materialData = new SMaterialData;


			xml_node<> *shaderNode = material->first_node("shader");
			xml_node<> *fileNameNode = shaderNode->first_node("fileName");
			xml_node<> *vsEnterPointNode = shaderNode->first_node("vsEnterPoint");
			xml_node<> *psEnterPointNode = shaderNode->first_node("psEnterPoint");
			if (Null(shaderNode) || Null(fileNameNode) || Null(vsEnterPointNode) || Null(psEnterPointNode))
			{
				Log_Err("load material shader error, file %s, batch %d", materialFileName.c_str(), index);
				return false;
			}

			buffer << fileNameNode->value();
			buffer >> materialData->shaderFile;
			buffer.clear();

			buffer << vsEnterPointNode->value();
			buffer >> materialData->vsEnterPoint;
			buffer.clear();

			buffer << psEnterPointNode->value();
			buffer >> materialData->psEnterPoint;
			buffer.clear();

			buffer << index;
			buffer >> materialData->identifer;
			buffer.clear();

			for (int i = 0; i < CONST_MAX_TEXTURE_NUM; ++i)
			{
				if (CONST_ALL_TEXTURE_POS_NAMES[i].name != "")
				{
					xml_node<> *node = material->first_node(CONST_ALL_TEXTURE_POS_NAMES[i].name.c_str());
					if (Not_Null(node))
					{
						buffer<<node->value();
						buffer >> mapName;
						materialData->texturePath[CONST_ALL_TEXTURE_POS_NAMES[i].index] = resourcePath + mapName;
						buffer.clear();
					}
					else{
						materialData->texturePath[CONST_ALL_TEXTURE_POS_NAMES[i].index] = "";
					}
				}
			}
			
			materialGroup.push_back(materialData);
		}
	}
	catch (exception e)
	{
		Log_Err("open file %s exception occur.", materialFileName.c_str());
		return false;
	}
	return true;
}