#ifndef MODEL_DISPLAY11_HEADER__
#define MODEL_DISPLAY11_HEADER__

#include<d3d11.h>
#include<D3DX11tex.h>
#include"ty_model_reader.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

class BaseModel
{
public:
	BaseModel() :vb_(nullptr),
		ib_(nullptr),
		offset_(0){}

	virtual ~BaseModel(){}

	inline void setContext(ID3D11DeviceContext* im)
	{
		this->im_ = im;
	}
	bool create(
		ID3D11Device* pd3dDevice,
		ModelData& data);


	void render(
		ID3D11DeviceContext* mD3dImmediateContext);

	void Release()
	{
		Safe_Release(vb_);
		Safe_Release(ib_);
		for (int i = 0; i < 5; ++i)
		{
			Safe_Release(texs_[i]);
		}
	}



private:
	ID3D11Buffer* vb_;
	ID3D11Buffer* ib_;
	UINT          stride_;
	UINT          offset_;
	UINT          indexCount_;
	ID3D11ShaderResourceView *texs_[5];
	ID3D11DeviceContext*      im_;
	group_info11              info_;
};




#endif