#ifndef MODEL_DISPLAY11_HEADER__
#define MODEL_DISPLAY11_HEADER__

#include<vector>
#include<d3d11.h>
#include<D3DX11tex.h>

#include"ty_model_reader.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

struct SubModel
{
	int							   texCount_;
	int                            indexOffset_;
	int                            indexCount_;
	std::vector<ID3D11ShaderResourceView*>  textures_;
};

class BaseModel
{
public:
	BaseModel() :vb_(nullptr),
		ib_(nullptr)
		{}

	virtual ~BaseModel(){}

	bool create(
		ID3D11Device* pd3dDevice,
		ModelData& data);


	void render(
		ID3D11DeviceContext* mD3dImmediateContext);

	void Release()
	{
		Safe_Release(vb_);
		Safe_Release(ib_);

		for (int i = 0; i < children_.size(); ++i)
		{
			SubModel& submodel = children_.at(i);
			for (int j = 0; j < submodel.texCount_; ++j)
			{
				Safe_Release(submodel.textures_[j]);
			}
		}
	}



private:
	ID3D11Buffer*			   vb_;
	ID3D11Buffer*			   ib_;
	unsigned int               stride_;
	unsigned int               offset_;
	unsigned int               childCount_;
	std::vector<SubModel>      children_;
};





#endif