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

	if (FAILED(dev->CreateBuffer(&vbd, &vdata, &vb_)))
	{
		assert(0);
		return false;
	}

	//ib
	ibd = vbd;
	ibd.ByteWidth = sizeof(short)*data.primtives_.indices_.size();;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ZeroMemory(&idata, sizeof(idata));
	idata.pSysMem = &data.primtives_.indices_[0];

	if (FAILED(dev->CreateBuffer(&ibd, &idata, &ib_)))
	{
		assert(0);
		return false;
	}

	//render batch
	for (unsigned int i = 0; i < data.groups_.size(); ++i)
	{
		SRenderGroupInfo* pGI = data.groups_[i];
		SubBatch batch;

		batch.SetRenderBatch(pGI->indexOffset_, pGI->indexCount_);

		if (data.materials_.size()>0 )
		{
			batch.SetMaterial(dev, data.materials_[pGI->materialID]);
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

	for (unsigned int i = 0; i < childCount_; ++i)
	{
		children_[i].Render(context);
	}
	int a = 0;
}


void SubBatch::SetMaterial(ID3D11Device* dev, SMaterialData* pMaterial)
{
	ResourceMgr* mgr = ResourceMgr::GetSingletonPtr();

	this->texCount_ = pMaterial->texCount;

	if (texCount_ > 0)
	{
		refTextures_ = new ID3D11ShaderResourceView*[texCount_];
		for (unsigned int i = 0; i < texCount_; ++i)
		{
			D3DX11CreateShaderResourceViewFromFileA(dev, pMaterial->texturePath[i].c_str(),
				nullptr, nullptr, &refTextures_[i], nullptr);
		}

	}


}