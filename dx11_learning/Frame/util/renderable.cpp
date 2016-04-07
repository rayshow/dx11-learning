#include<assert.h>

#include"renderable.h"
#include"res_mgr.h"

using namespace ul;

bool BaseModel::Create(
	ID3D11Device* dev,
	SModelData& data)
{
	HRESULT hr;
	D3D11_BUFFER_DESC vbd;
	D3D11_BUFFER_DESC ibd;
	D3D11_SUBRESOURCE_DATA vdata, idata;
	ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();


	//vb
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = data.primtives_.verticeNum_*data.primtives_.stride_;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	ZeroMemory(&vdata, sizeof(vdata));
	vdata.pSysMem = &data.primtives_.verticeBuffer_[0];

	Null_Return_False((vb_ = mgr->CreateBuffer(vbd, &vdata)));


	//ib
	ibd = vbd;
	ibd.ByteWidth = sizeof(short)*data.primtives_.indices_.size();;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ZeroMemory(&idata, sizeof(idata));
	idata.pSysMem = &data.primtives_.indices_[0];

	Null_Return_False((ib_ = mgr->CreateBuffer(ibd, &idata)));

	//material 
	renderParameters_.reserve(data.materials_.size());

	for (ulUint i = 0; i < data.materials_.size(); ++i)
	{
		SMaterialData *pMaterial = data.materials_[i];
		SRenderParameter* pParameter = new SRenderParameter();
		pParameter->srvCount_ = pMaterial->texCount;

		if (pMaterial->texCount > 0)
		{
			pParameter->srvs_ = new ID3D11ShaderResourceView*[pMaterial->texCount];
			for (int j = 0; j < pMaterial->texCount; ++j)
			{
				pParameter->srvs_[j] = mgr->CreateTextureFromFile(pMaterial->texturePath[j]);
			}
		}
		else{
			pParameter->srvs_ = nullptr;
		}
		renderParameters_.push_back(pParameter);
	}

	//render batch
	children_.reserve(data.groups_.size());
	for (ulUint i = 0; i < data.groups_.size(); ++i)
	{
		SRenderGroupInfo* pGI = data.groups_[i];
		SubBatch batch;

		batch.SetRenderBatch(pGI->indexOffset_, pGI->indexCount_);

		if (data.materials_.size()>0 )
		{
			batch.SetParameter(renderParameters_[ pGI->materialID] );
		}
			
		children_.push_back(batch);
	}

	childCount_ = children_.size();
	stride_ = data.primtives_.stride_;
	offset_ = 0;
	return true;
}
void BaseModel::Render(ID3D11DeviceContext* context)
{
	assert(context != 0);
	context->IASetVertexBuffers(0, 1, &vb_, &stride_, &offset_);
	context->IASetIndexBuffer(ib_, DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (ulUint i = 0; i < childCount_; ++i)
	{
		children_[i].Render(context);
	}
	int a = 0;
}

