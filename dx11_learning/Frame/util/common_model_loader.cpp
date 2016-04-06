
#include"common_model_loader.h"

using namespace ul;

bool CommonModelLoader::LoadFile(const std::string& fileName
	, SModelData& data)
{
	Assimp::Importer import;
	const aiScene* pScene = import.ReadFile(fileName.c_str(),
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

	Null_Return_False_With_Msg( pScene, "read model file %s error: %s.",
		fileName.c_str(), import.GetErrorString() );

	if ( Zero(pScene->mNumMeshes) || Null(pScene->mMeshes[0]) )
	{
		Log_Err("read model file %s error", fileName.c_str());
		return false;
	}

	if ( pScene->mMeshes[0]->mFaces[0].mNumIndices!=3)
	{
		Log_Err("frame only support triangle primitive! from file: %s", fileName.c_str());
		return false;
	}

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

	ulUint bufferOffset = 0;
	ulUint indexOffset = 0;
	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
	{
		aiMesh* mesh = pScene->mMeshes[i];

		//render group info
		SRenderGroupInfo *pGroup = new SRenderGroupInfo();
		pGroup->materialID = mesh->mMaterialIndex;
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

	for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
	{
		loadMaterial(pScene->mMaterials[i]);
	}
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
				memcpy(pVertex->uv_,  &mesh->mTextureCoords[i], sizeof(float) * 2);
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
				memcpy(pVertex->uv_, &mesh->mTextureCoords[i], sizeof(float) * 2);
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


bool CommonModelLoader::loadIndiceData(aiMesh* mesh, ulUshort* buffer)
{
	for (ulUint i = 0; i < mesh->mNumFaces; ++i)
	{
		buffer[3 * i]   = mesh->mFaces[i].mIndices[0];
		buffer[3 * i+1] = mesh->mFaces[i].mIndices[1];
		buffer[3 * i+2] = mesh->mFaces[i].mIndices[2];
	}
	return true;
}

bool CommonModelLoader::loadMaterial(const aiMaterial *material)
{
	return true;
}