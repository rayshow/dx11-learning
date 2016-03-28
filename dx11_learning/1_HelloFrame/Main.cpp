#include<Windows.h>
#include<memory>

#include"util/tools.h"
#include"Application.h"
#include"res_mgr.h"

using namespace ul;
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
		Null_Return_Void( (fullScreenVertex_ = mgr->CreateVertexShader("main.hlsl", "VS_FullScreenProcess", "vs_5_0")) );
		Null_Return_Void( (fullScreenPixel_ = mgr->CreatePixelShader("main.hlsl", "PS_FullScreenProcess", "ps_5_0")) );
		Null_Return_Void( (texture_ = mgr->CreateTextureFromFile("../Res/test.dds") ) );
		
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

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		context->RSSetState(resterState_);
		context->PSSetSamplers(0, 1, &pointSampler_);
		context->PSSetShaderResources(0, 1, &texture_);
		context->VSSetShader(fullScreenVertex_, nullptr, 0);
		context->PSSetShader(fullScreenPixel_, nullptr, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->Draw(3, 0);
	};

	virtual void Exit()
	{
	};
private:
	ID3D11VertexShader*  fullScreenVertex_;
	ID3D11PixelShader*   fullScreenPixel_;
	ID3D11ShaderResourceView *texture_;
	ID3D11SamplerState   *pointSampler_;
	ID3D11RasterizerState* resterState_;
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