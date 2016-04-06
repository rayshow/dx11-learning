#ifndef UL_SKYBOX_HEADER__
#define UL_SKYBOX_HEADER__

#include<d3d11.h>

class Environmentable
{
public:
	virtual ID3D11ShaderResourceView* GetSpecularEnvmap() = 0;
	virtual ID3D11ShaderResourceView* GetDiffuseEnvmap() = 0;
};





class SkyBox
{

};







#endif