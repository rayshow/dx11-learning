#include<assert.h>

#include"ty_model_display11.h"


static void getInputLayoutFromVertexType()
{

}


bool BaseModel::create(
	ID3D11Device* pd3dDevice,
	ModelData& data)
{
	HRESULT hr;
	D3D11_BUFFER_DESC vbd;
	D3D11_BUFFER_DESC ibd;
	D3D11_SUBRESOURCE_DATA vdata, idata;

	//vb
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = data.primData.verticeNum_*data.primData.stride_;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	ZeroMemory(&vdata, sizeof(vdata));
	vdata.pSysMem = &data.primData.verticeBuffer_[0];
	if (FAILED(pd3dDevice->CreateBuffer(&vbd, &vdata, &vb_)))
	{
		assert(0);
		return false;
	}

	//ib
	ibd = vbd;
	ibd.ByteWidth = sizeof(short)*data.primData.indices_.size();;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ZeroMemory(&idata, sizeof(idata));
	idata.pSysMem = &data.primData.indices_[0];
	
	if (FAILED(pd3dDevice->CreateBuffer(&ibd, &idata, &ib_)))
	{
		assert(0);
		return false;
	}

	//texture0
	for (int i = 0; i < data.groups_.size(); ++i)
	{
		group_info11* gi = data.groups_[i];
		SubModel subModel;
		subModel.indexCount_ = gi->indexCount_;
		subModel.indexOffset_ = gi->indexOffset_;
		subModel.texCount_ = gi->material_.texCount;
		subModel.textures_.resize(subModel.texCount_);
		for (int j = 0; j < subModel.texCount_; ++j)
		{
			D3DX11CreateShaderResourceViewFromFileA(pd3dDevice, gi->material_.texturePath[j].c_str()
			, nullptr, nullptr, &subModel.textures_[j], nullptr);
		}
			
		children_.push_back(subModel);
	}

	childCount_ = children_.size();
	stride_ = data.primData.stride_;
	offset_ = 0;
	return true;
}
void BaseModel::render(ID3D11DeviceContext* mD3dImmediateContext)
{
	assert(mD3dImmediateContext != 0 );
	mD3dImmediateContext->IASetVertexBuffers(0, 1, &vb_, &stride_, &offset_);
	mD3dImmediateContext->IASetIndexBuffer(ib_, DXGI_FORMAT_R16_UINT, 0);
	mD3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (int i = 0; i < childCount_; ++i)
	{
		mD3dImmediateContext->PSSetShaderResources(0, children_[i].texCount_, &children_[i].textures_[0]);
		mD3dImmediateContext->DrawIndexed(children_[i].indexCount_, children_[i].indexOffset_, 0);
	}
	int a = 0;
}