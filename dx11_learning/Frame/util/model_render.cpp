
#include"model_render.h"

#include"res_mgr.h"

using namespace ul;

bool ModelRender::LoadMesh(const std::string& fileName)
{
	Assimp::Importer import;
	const aiScene* pScene = import.ReadFile(fileName.c_str(),
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

	Null_Return_False_With_Msg( pScene, "read model file %s error: %s.",
		fileName.c_str(), import.GetErrorString() );

	submeshs_.reserve(pScene->mNumMeshes);
	textures_.reserve(pScene->mNumMaterials);
	sumMeshCount_ = pScene->mNumMeshes;
	for (ulUint i = 0; i < pScene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = pScene->mMeshes[i];
		initMesh(mesh);
	}
}

void ModelRender::Render(ID3D11DeviceContext* context)
{
	assert(context != 0);
	for (ulUint i = 0; i < sumMeshCount_; ++i)
	{
		SubMesh* mesh = submeshs_[i];
		context->IASetVertexBuffers(0, 1, &mesh->vb_, &mesh->stride_, &mesh->offset_);
		context->IASetIndexBuffer(mesh->ib_, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->DrawIndexed(mesh->indexCount_, 0, 0);
	}
}

bool ModelRender::initMesh(const aiMesh* aiMesh)
{
	const aiVector3D zero(0,0,  0);

	SubMesh *subMesh = new SubMesh();
	ResourceMgr *mgr = ResourceMgr::GetSingletonPtr();
	std::vector<VertexXyznuv> verticeVec;
	std::vector<ulUint> indiceVec;

	subMesh->materialIndex_ = aiMesh->mMaterialIndex;
	verticeVec.resize(aiMesh->mNumVertices);
	for (ulUint i = 0; i < aiMesh->mNumVertices; ++i)
	{
		VertexXyznuv *vertex = &verticeVec[i];
		const aiVector3D* pos = &(aiMesh->mVertices[i]);
		const aiVector3D* normal = &(aiMesh->mNormals[i]);
		const aiVector3D* coord = (aiMesh->HasTextureCoords(i) ? &aiMesh->mTextureCoords[0][i] : &zero);
		memcpy(vertex->pos_, pos, sizeof(float)* 3);
		memcpy(vertex->normal_, normal, sizeof(float)* 3);
		memcpy(vertex->coord_, coord, sizeof(float)*2);
	}

	indiceVec.reserve(aiMesh->mNumFaces * 3);
	for (ulUint i = 0; i < aiMesh->mNumFaces; ++i)
	{
		const aiFace* face = &aiMesh->mFaces[i];
		assert(face->mNumIndices == 3);
		indiceVec.push_back(face->mIndices[0]);
		indiceVec.push_back(face->mIndices[1]);
		indiceVec.push_back(face->mIndices[2]);
	}

	ulUint stride = sizeof(VertexXyznuv);
	subMesh->vb_ = mgr->CreateVertexBuffer(&verticeVec.front(), stride*verticeVec.size());
	subMesh->ib_ = mgr->CreateIndiceBuffer(&indiceVec.front(), sizeof(ulUint)* indiceVec.size());
	subMesh->stride_ = stride;
	subMesh->offset_ = 0;
	subMesh->indexCount_ = indiceVec.size();
	if (Null(subMesh->vb_) || Null(subMesh->vb_))
	{
		delete subMesh;
		return nullptr;
	}
	submeshs_.push_back(subMesh);
}

bool ModelRender::initMaterial(const aiMaterial *material)
{
	return true;
}