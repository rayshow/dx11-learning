#include<assert.h>

#include"MeshRender.h"
#include"../resource/ResourceMgr.h"
#include"../scene/SceneManager.h"

using namespace ul;

bool StaticMeshRender::Create(ID3D11Device* pd3dDevice, const SModelData& data)
{
	D3D11_BUFFER_DESC vbd;
	D3D11_BUFFER_DESC ibd;
	D3D11_SUBRESOURCE_DATA vdata, idata;
	ResourceMgr* pResourceMgr = ResourceMgr::GetSingletonPtr();

	//vb
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = data.primtives_.verticeNum_*data.primtives_.stride_;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	ZeroMemory(&vdata, sizeof(vdata));
	vdata.pSysMem = &data.primtives_.verticeBuffer_[0];
	Null_Return_False((vb_ = pResourceMgr->CreateBuffer(vbd, &vdata)));

	//ib
	ibd = vbd;
	ibd.ByteWidth = sizeof(ulUint)*data.primtives_.indices_.size();;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ZeroMemory(&idata, sizeof(idata));
	idata.pSysMem = &data.primtives_.indices_[0];
	Null_Return_False((ib_ = pResourceMgr->CreateBuffer(ibd, &idata)));

	vertexType_ = data.primtives_.type_;
	meshFileName_ = data.sourceFile_;

	//render parameters
	materials_.reserve(data.materials_.size());
	for (ulUint i=0; i < data.materials_.size(); ++i)
	{
		SMaterialData*   pMaterialData = data.materials_[i];
		SRenderData*     pRenderData = new SRenderData();
		memset(pRenderData, 0, sizeof(SRenderData));
		//shader
		this->setShaderFile(pResourceMgr, pMaterialData->shaderFile, pRenderData);
		//texture
		for (int j = 0; j < CONST_MAX_TEXTURE_NUM; ++j)
		{
			if (pMaterialData->texturePath[j] != "")
			{
				Null_Return_False( (pRenderData->shaderSRVs_[j] = pResourceMgr->CreateTextureFromFile(pMaterialData->texturePath[j])) );
			}
		}

		materials_.push_back(pRenderData);
	}
	//inputlayout
	this->setInputLayout(pResourceMgr);


	//render batch
	children_.reserve(data.groups_.size());
	for (ulUint i = 0; i < data.groups_.size(); ++i)
	{
		SRenderGroupInfo* pGroup = data.groups_[i];
		StaticMeshPart part;
		part.SetRenderBatch(pGroup->indexOffset_, pGroup->indexCount_);
		part.SetMaterial(materials_.at(pGroup->materialID));
		children_.push_back(part);
	}

	childCount_ = children_.size();
	stride_ = data.primtives_.stride_;
	offset_ = 0;
	return true;
}


void StaticMeshRender::SetEffect(const string& fileName)
{
	ResourceMgr* pResourceMgr = ResourceMgr::GetSingletonPtr();
	materials_.reserve(children_.size());
	for (int i = 0; i < children_.size(); ++i)
	{
		SRenderData* pRenderData = new SRenderData;
		memset(pRenderData, 0, sizeof(SRenderData));
		this->setShaderFile(pResourceMgr, fileName, pRenderData);
	}
	this->setInputLayout(pResourceMgr);

}



void StaticMeshRender::setShaderFile(ResourceMgr* pResourceMgr, const string& shaderFileName, SRenderData* pRenderData)
{	
	ID3DX11Effect* pEffect = nullptr;
	ID3DX11EffectTechnique* pTechnique = nullptr;
	D3DX11_TECHNIQUE_DESC techDesc;

	Null_Return_Void((pEffect = pResourceMgr->LoadEffectFromCompileFile(shaderFileName.c_str())));
	Null_Return_Void((pTechnique = pEffect->GetTechniqueByName("Default")));
	pTechnique->GetDesc(&techDesc);
	pRenderData->passNum_ = techDesc.Passes;
	pRenderData->passes_.reserve(pRenderData->passNum_);
	for (ulUint j = 0; j < pRenderData->passNum_; ++j)
	{
		pRenderData->passes_.push_back(pTechnique->GetPassByIndex(j));
		hasRenderPass_ = true;
	}
}


void StaticMeshRender::setInputLayout(ResourceMgr* pResourceMgr)
{
	if (hasRenderPass_)
	{
		Vertex_GetInputDescByType(vertexType_, desc_, &descCount_);
		D3DX11_PASS_DESC passDesc;
		materials_.at(0)->passes_.at(0)->GetDesc(&passDesc);
		vertexLayout_ = pResourceMgr->CreateInputLayoutFromPassDesc(desc_, descCount_, passDesc);
	}
}

void StaticMeshRender::Render(ID3D11DeviceContext* context)
{
	assert(context != 0);
	assert(hasRenderPass_);
	context->IASetVertexBuffers(0, 1, &vb_, &stride_, &offset_);
	context->IASetIndexBuffer(ib_, DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(vertexLayout_);

	for (ulUint i = 0; i < childCount_; ++i)
	{
		children_[i].Render(context);
	}
}


void StaticMeshPart::Render(ID3D11DeviceContext* context)
{
	for (ulUint i = 0; i < refMaterial_->passNum_; ++i)
	{
		refMaterial_->passes_[i]->Apply(0, context);
		context->DrawIndexed(indexCount_, indexOffset_, 0);
	}
}