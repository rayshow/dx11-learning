#include<rapidxml.hpp>
#include<rapidxml_utils.hpp>
#include<sstream>

#include"Environment.h"
#include"../resource/ResourceMgr.h"
#include"../scene/SceneManager.h"

using namespace ul;
using namespace rapidxml;
using namespace std;

bool Environment::setEnvmap(
	const std::string& diffuseName,
	const std::string& specularName,
	const std::string& intergeFileName)
{
	ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();

	Null_Return_False_With_Msg(
		(environmentMaps_[0] = mgr->CreateTextureFromFile(diffuseName)),
		"create diffuse env map from file:%s  error.",
		diffuseName.c_str()
		);

	Null_Return_False_With_Msg(
		(environmentMaps_[1] = mgr->CreateTextureFromFile(specularName)),
		"create specular env map from file:%s  error.",
		specularName.c_str()
		);

	Null_Return_False_With_Msg(
		(environmentMaps_[2] = mgr->CreateTextureFromFile(intergeFileName)),
		"create interge map from file:%s  error.",
		intergeFileName.c_str()
		);

	return true;
}


void Environment::Apply()
{
	ID3D11DeviceContext* context = ResourceMgr::GetSingletonPtr()->GetContext();
	context->PSSetShaderResources(5, 3, environmentMaps_);
}

bool Environment::Initialize(const string& fileName)
{
	stringstream buffer;
	string irridianceMapName = "";
	string specularMapName = "";
	string lookupBrdfMapName = "";
	ulFloat exposure = 1;

	string baseResourcePath = ResourceMgr::GetSingletonPtr()->GetResourceBasePath();

	string fullPathName = baseResourcePath + fileName;
	try{
		file<> fl(fullPathName.c_str());
		xml_document<> doc;
		doc.parse<0>(fl.data());

		//nodelessVisual
		xml_node<> *environment = nullptr;
		if ((environment = doc.first_node("environment")) == nullptr)
		{
			Log_Err("can't find environment data in %s", fullPathName.c_str());
			return false;
		}

		xml_node<> *irridianceMapNode = environment->first_node("irridianceMap");
		xml_node<> *specularMapNode = environment->first_node("specularMap");
		xml_node<> *lookupMapNode = environment->first_node("lookupMap");
		xml_node<> *exposureNode = environment->first_node("exposure");
		if (Null(irridianceMapNode) || Null(specularMapNode) || Null(lookupMapNode) || Null(exposureNode))
		{
			Log_Err("can't find environment data in %s, some node lost", fullPathName.c_str());
			return false;
		}

		buffer << irridianceMapNode->value();
		buffer >> irridianceMapName;
		irridianceMapName = baseResourcePath + irridianceMapName;
		buffer.clear();

		buffer << specularMapNode->value();
		buffer >> specularMapName;
		specularMapName = baseResourcePath + specularMapName;
		buffer.clear();

		buffer << lookupMapNode->value();
		buffer >> lookupBrdfMapName;
		lookupBrdfMapName = baseResourcePath + lookupBrdfMapName;
		buffer.clear();
		
		buffer << irridianceMapNode->value();
		buffer >> exposure;
		buffer.clear();
	}
	catch (exception ex){
		Log_Err("load environment exception:%s form file: %s", ex.what(), fullPathName.c_str() );
		return false;
	}

	this->setEnvmap(irridianceMapName, specularMapName, lookupBrdfMapName);
	this->exposure_ = exposure;
	return true;
}

bool SkyBox::createRenderData()
{
	SModelData data;
	ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();
	data.primtives_.stride_ = sizeof(SVertexXyzNuv);
	data.primtives_.indiceNum_ = 36;
	data.primtives_.type_ = eVertex_XYZNUV;
	data.primtives_.verticeNum_ = 8;
	data.primtives_.verticeBuffer_.resize(data.primtives_.verticeNum_*data.primtives_.stride_);
	data.primtives_.indices_.resize(36);

	SVertexXyzNuv* buffer = static_cast<SVertexXyzNuv*>((void*)&data.primtives_.verticeBuffer_[0]);
	Vertex_FillXyznuv(buffer[0], -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex_FillXyznuv(buffer[1], -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex_FillXyznuv(buffer[2], 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex_FillXyznuv(buffer[3], 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex_FillXyznuv(buffer[4], 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex_FillXyznuv(buffer[5], 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex_FillXyznuv(buffer[6], -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex_FillXyznuv(buffer[7], -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	data.primtives_.indices_ = {
		0, 1, 2,
		2, 3, 0,
		3, 2, 5,
		5, 4, 3,
		4, 5, 6,
		6, 7, 4,
		7, 6, 1,
		1, 0, 7,
		7, 0, 3,
		3, 4, 7,
		1, 6, 5,
		5, 2, 1
	};
	//primitive data
	SRenderGroupInfo *pGroup = new SRenderGroupInfo;
	pGroup->indexCount_ = data.primtives_.indiceNum_;
	pGroup->indexOffset_ = 0;
	pGroup->materialID = 0;
	data.groups_.push_back(pGroup);

	//material
	SMaterialData* materialData = new SMaterialData;
	materialData->identifer = "skybox";
	materialData->shaderFile = mgr->GetResourceBasePath()+"fx/skybox.fxo";
	data.materials_.push_back(materialData);

	box_.Create(mgr->GetDevice(), data);
	ModelData_Free(data);
	return true;
}

