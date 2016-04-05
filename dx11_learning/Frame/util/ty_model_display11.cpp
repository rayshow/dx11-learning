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
	memcpy(&info_, data.primData.groups[1], sizeof(group_info11));

	stride_ = sizeof(VertexXyznuviiiwwtb);
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = data.primData.verticeNum_*stride_;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	
	
	ZeroMemory(&vdata, sizeof(vdata));
	vdata.pSysMem = data.primData.verticesData_;

	if (FAILED(pd3dDevice->CreateBuffer(&vbd, &vdata, &vb_)))
	{
		assert(0);
		return false;
	}

	ibd = vbd;
	indexCount_ = data.primData.indices.size();
	ibd.ByteWidth = sizeof(short)*indexCount_;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ZeroMemory(&idata, sizeof(idata));
	idata.pSysMem = &data.primData.indices[0];
	
	if (FAILED(pd3dDevice->CreateBuffer(&ibd, &idata, &ib_)))
	{
		assert(0);
		return false;
	}


	D3DX11CreateShaderResourceViewFromFileA(pd3dDevice, data.matData.diffuseTexMap.c_str(), nullptr, nullptr, &texs_[0], nullptr);
	D3DX11CreateShaderResourceViewFromFileA(pd3dDevice, data.matData.normalTexMap.c_str(), nullptr, nullptr, &texs_[1], nullptr);
	D3DX11CreateShaderResourceViewFromFileA(pd3dDevice, data.matData.specularTexMap.c_str(), nullptr, nullptr, &texs_[2], nullptr);
	return true;
}
void BaseModel::render(ID3D11DeviceContext* mD3dImmediateContext)
{
	assert(mD3dImmediateContext != 0 );
	mD3dImmediateContext->IASetVertexBuffers(0, 1, &vb_, &stride_, &offset_);
	mD3dImmediateContext->IASetIndexBuffer(ib_, DXGI_FORMAT_R16_UINT, 0);
	mD3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mD3dImmediateContext->PSSetShaderResources(0, 3, texs_);
	
	//mD3dImmediateContext->DrawIndexed(indexCount_, 0, 0);
	mD3dImmediateContext->DrawIndexed(info_.indiceCount, info_.startIndex, 0);
	
	int a = 0;
	
}