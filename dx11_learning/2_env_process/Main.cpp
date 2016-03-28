#include<Windows.h>
#include<memory>

#include"util/tools.h"
#include"Application.h"
#include"res_mgr.h"
#include<model_render.h>
#include<D3DX10Math.h>

using namespace ul;


struct CB_PerFrame
{
	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX project;
	D3DXMATRIX convoltViews[6];
	float      level;
	float      pad[3];
};

class Lession1_Frame :public Application
{
public:
	virtual ~Lession1_Frame(){}
public:
	virtual void WindowResize(int width, int height,
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{

	};

	virtual void InitResource(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		ResourceMgr *mgr = ResourceMgr::GetSingletonPtr();
	
		Null_Return_Void(( ball_ = mgr->CreateModelFromFile("../res/mesh/sphere.x")));
		Null_Return_Void( (texture_ = mgr->CreateTextureFromFile("../Res/test.dds") ) );
		Null_Return_Void((
			commonInputVertexPass_ = mgr->CreateVertexShaderAndInputLayout(
			"main.hlsl", "VS_RenderNormalMesh", "vs_5_0",
			VertexXyznuv_Layout, ARRAYSIZE(VertexXyznuv_Layout), &commonInputLayout_)
		));
		Null_Return_Void((commonInputPixelPass_ = mgr->CreatePixelShader("main.hlsl", "PS_ResultPass", "ps_5_0")));
		Null_Return_Void((perframeBuffer_ = mgr->CreateConstantBuffer(sizeof(CB_PerFrame))));

		D3D11_SAMPLER_DESC SamDesc;
		SamDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		SamDesc.MipLODBias = 0.0f;
		SamDesc.MaxAnisotropy = 1;
		SamDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 1.0;
		SamDesc.MinLOD = 0;
		SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
		SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		Null_Return_Void( (pointSampler_ = mgr->CreateSamplerState(SamDesc)) );   //µã

		D3D11_RASTERIZER_DESC rester;
		memset(&rester, 0, sizeof(rester));
		rester.CullMode = D3D11_CULL_NONE;
		rester.AntialiasedLineEnable = true;
		rester.DepthBias = 0;
		rester.DepthBiasClamp = 0;
		rester.DepthClipEnable = true;
		rester.SlopeScaledDepthBias = 0;
		rester.FillMode = D3D11_FILL_SOLID;
		rester.FrontCounterClockwise = false;
		rester.MultisampleEnable = false;
		rester.ScissorEnable = false;
		Null_Return_Void( (resterState_ = mgr->CreateRasterState(rester)) );
	};

	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		context->IASetInputLayout(commonInputLayout_);
		context->RSSetState(resterState_);
		context->PSSetSamplers(0, 1, &pointSampler_);

		D3DXMATRIX world, view, project;
		D3DXMatrixIdentity(&world);
		D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(0, 0, -4), &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 1, 0));

		D3DXMatrixPerspectiveFovLH(&project, 3.14 / 4, 1, 0.1, 1000);
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		memset(&MappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		Fail_Return_Void(context->Map(perframeBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));

		CB_PerFrame* pConstants = (CB_PerFrame*)MappedResource.pData;
		D3DXMatrixTranspose(&pConstants->world, &world);
		D3DXMatrixTranspose(&pConstants->view, &view);
		D3DXMatrixTranspose(&pConstants->project, &project);
		for (int j = 0; j< 6; ++j)
		{
			D3DXMatrixTranspose(&pConstants->convoltViews[j], &view);
		}
		context->Unmap(perframeBuffer_, 0);
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);

		context->VSSetConstantBuffers(0, 1,&perframeBuffer_);
		context->PSSetConstantBuffers(0, 1, &perframeBuffer_);
		context->PSSetShaderResources(0, 1, &texture_);
		context->VSSetShader(commonInputVertexPass_, nullptr, 0);
		context->PSSetShader(commonInputPixelPass_, nullptr, 0);
		ball_->Render(context);
	};

	virtual void Exit()
	{
	};
private:
	ID3D11VertexShader*  fullScreenVertex_;
	ID3D11PixelShader*   fullScreenPixel_;
	ID3D11VertexShader*  commonInputVertexPass_;
	ID3D11InputLayout*   commonInputLayout_;
	ID3D11PixelShader*   commonInputPixelPass_;
	ID3D11Buffer*        perframeBuffer_;

	ID3D11ShaderResourceView *texture_;
	ID3D11SamplerState   *pointSampler_;
	ID3D11RasterizerState* resterState_;
	ModelRender          * ball_;
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR pScmdline, int iCmdshow)
{
	ul::OpenConsoleAndDebugLeak();

	Log_Info("hello");
	//Utils::SetBreakPointAtMemoryLeak(154);
	Lession1_Frame  *app = new Lession1_Frame;
	if (!app)
	{
		return 0;
	}

	// Initialize and run the system object.
	if (app->Initialize(800, 600))
	{
		app->Run();
	}

	// Shutdown and release the system object.
	app->Shutdown();
	delete app;
	app = 0;

	return 0;
}