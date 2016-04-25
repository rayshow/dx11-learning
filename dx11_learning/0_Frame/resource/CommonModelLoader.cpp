#include"CommonModelLoader.h"

#include<rapidxml.hpp>
#include<rapidxml_utils.hpp>
#include<sstream>
#include<exception>
#include<xnamath.h>


using namespace ul;
using namespace rapidxml;
using namespace std;

static EMaterialParameterType MaterialParameterType_FromString(const std::string& type)
{
	if (type == "texture")
		return eMaterialParameter_Path;
	if (type == "vector")
		return eMaterialParameter_Vector;
	if (type == "float")
		return eMaterialParameter_Float;
	if (type == "int")
		return eMaterialParameter_Int;
	return eMaterialParameter_Unkown;
};



static void MaterialParameter_ResovleValue(SMaterialParameter* parameter,
	const std::string content, std::stringstream& buffer)
{
	XMFLOAT4 vector;

	switch (parameter->type)
	{
	case eMaterialParameter_Path:
		parameter->size = content.length() + 1;
		parameter->value = new char[parameter->size];
		memcpy(parameter->value, content.c_str(), parameter->size);
		break;
	case eMaterialParameter_Vector:
		buffer << content;
		buffer >> vector.x  >> vector.y  >> vector.z  >> vector.w;
		parameter->size = sizeof(vector);
		parameter->value = new XMFLOAT4[1];
		memcpy(parameter->value, &vector, sizeof(vector));
		break;
	case eMaterialParameter_Float:
		float fvalue;
		buffer << content;
		buffer >> fvalue;
		parameter->value = new float[1];
		parameter->size = sizeof(float);
		memcpy(parameter->value, &fvalue, sizeof(float));
		break;
	case eMaterialParameter_Int:
		int ivalue;
		buffer << content;
		buffer >> ivalue;
		parameter->value = new int[1];
		parameter->size = sizeof(int);
		memcpy(parameter->value, &ivalue, sizeof(int));
		break;
	case eMaterialParameter_Unkown:
		assert(0);
		break;
	}
	buffer.clear();
}


bool CommonModelLoader::LoadFile(const std::string resourcePath, const std::string& fileName
	, SModelData& data)
{
	const std::string meshFileName = resourcePath + fileName;

	Assimp::Importer import;
	const aiScene* pScene = import.ReadFile(meshFileName.c_str(),
		aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

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
	std::string materialName = meshFileName.substr(0, off) + ".visual";

	ifstream fin(materialName, ios::binary);
	if (!fin.is_open())
	{
		Log_Info("no visual file found of %s.", meshFileName.c_str());
		return true;
	}

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



	try{
		file<> fl(materialFileName.c_str());

		xml_document<> doc;
		doc.parse<0>(fl.data());

		//nodelessVisual
		xml_node<> *pVisualNode = nullptr;
		if ((pVisualNode = doc.first_node("visual")) == nullptr)
		{
			Log_Err("can't find visual node in %s", materialFileName.c_str());
			return false;
		}

		xml_node<> *pRenderSet = pVisualNode->first_node("renderset");
		if (Null(pRenderSet))
		{
			Log_Err("can't find renderset node in %s", materialFileName.c_str());
			return false;
		}

		xml_node<> *pMaterialNode = pRenderSet->first_node("material");
		for (; pMaterialNode != nullptr; pMaterialNode = pMaterialNode->next_sibling("material"))
		{
			SMaterialData* pMaterialData = new SMaterialData;
			xml_node<> *pShaderNode = pMaterialNode->first_node("shader");
			buffer << pShaderNode->value();
			buffer >> mapName;
			pMaterialData->shaderFile = resourcePath + mapName;
			buffer.clear();
			
			xml_attribute<>* nameNode = nullptr;
			xml_attribute<>* typeNode = nullptr;
			SMaterialParameter *parameter = nullptr;
			//parameter
			xml_node<> *parameterNode = pMaterialNode->first_node("parameter");
			for (; parameterNode != nullptr; parameterNode = parameterNode->next_sibling("parameter"))
			{
				nameNode  = parameterNode->first_attribute("name");
				typeNode  = parameterNode->first_attribute("type");
				if (Null(nameNode) || Null(typeNode))
				{
					Log_Err("parameter attribute miss in file %s", materialFileName.c_str());
					return false;
				}
				parameter = new  SMaterialParameter;
				buffer << nameNode->value();
				buffer >> parameter->name;
				buffer.clear();
				buffer << typeNode->value();
				buffer >> mapName;
				parameter->type = MaterialParameterType_FromString(mapName);
				buffer.clear();
				buffer << parameterNode->value();
				buffer >> mapName;
				MaterialParameter_ResovleValue(parameter, mapName, buffer);
				pMaterialData->parameters.push_back(parameter);
			}
			materialGroup.push_back(pMaterialData);
		}
	}
	catch (exception e)
	{
		Log_Err("open file %s exception occur.", materialFileName.c_str());
		return false;
	}
	return true;
}