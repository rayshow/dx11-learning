#include<Windows.h>
#include<memory>

#include<gui/AntTweakBar.h>
#include<Application.h>

#include<render/PostProcess.h>

#include<xnamath.h>

using namespace ul;


struct CB_PerFrame
{
	XMFLOAT4X4  world;
	XMFLOAT4X4  worldViewProject;
	XMFLOAT4    camaraPos;
	unsigned    specularType;
	unsigned    irridianceType;
	unsigned    outputType;
	unsigned    padding;
};

struct CB_Coeffs
{
	XMFLOAT4   coeffs[9];
};


enum ESpecular_Type
{
	eSpecular_Ibl = 0,
	eSpecular_Realtime_Imp,
};

enum EIrridiance_Type
{
	eIrridiance_Ibl =0,
	eIrridiance_Sh,
};

enum EToneMapping_Type
{
	eTonemapping_Avg_Lumin =0,
	eTonemapping_Filmic,
};

enum EOutput_Type
{
	eOutput_LitColor,
	eOutput_Irridiance,
	eOutput_Specular,
	eOutput_Ao,
	eOutput_Roughness,
	eOutput_Materness,
	eOutput_Lukup,
	eOutput_Normal,
};

TwEnumVal SpecularEnums[] =
{
	{ eSpecular_Ibl, "specular ibl map" },
	{ eSpecular_Realtime_Imp, "importance sample reoughness" }
};

TwEnumVal IrridianceEnums[]=
{
	{ eIrridiance_Ibl, "specular ibl map" },
	{ eIrridiance_Sh, "importance sample reoughness" }
};

TwEnumVal OutputEnums[] =
{
	{ eOutput_LitColor, "lighting color" },
	{ eOutput_Normal, "normal " },
	{ eOutput_Irridiance, "irridiance color" },
	{ eOutput_Specular, "specular color" },
	{ eOutput_Ao, "ao color" },
	{ eOutput_Roughness, "roughness" },
	{ eOutput_Materness, "matelness" },
	{ eOutput_Lukup, "interge lukup " },
};

TwEnumVal TonemmappingEnums[] =
{
	{ eTonemapping_Avg_Lumin, "average luminance method" },
	{ eTonemapping_Filmic, "filmic method" }
};


class Lession1_Frame :public Application
{
public:
	virtual ~Lession1_Frame(){}
public:
	virtual bool InitResource(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		ResourceMgr *mgr = ResourceMgr::GetSingletonPtr();
		mgr->SetResourceBasePath("../res/");

		uiRotate_.w = 1;
		XMVECTOR axis = XMLoadFloat4(&XMFLOAT4(1, 0, 0, 0));
		XMVECTOR rotate = XMQuaternionRotationAxis(axis, -XM_PI / 2);
		XMStoreFloat4(&uiRotate_, rotate);

		//ui
		TwInit(TW_DIRECT3D11, this->GetDevicePtr());
		TwBar *bar = TwNewBar("TweakBar");
		TwDefine(" Main label='hello");
		int barSize[2] = { 200, 400 };

		uiSpecularType_ = eSpecular_Ibl;
		uiIrridianceType_ = eIrridiance_Ibl;
		uiTonemappingType_ = eTonemapping_Avg_Lumin;
		uiOutputType_ = eOutput_LitColor;
		uiExpoure_ = 2.0f;

		TwType specularTypes =    TwDefineEnum("SpecularType", SpecularEnums, 2);
		TwType irrianceTypes =    TwDefineEnum("IrridianceType", IrridianceEnums, 2);
		TwType outputTypes = TwDefineEnum("OutputType", OutputEnums, 8);
		TwType tonemappingTypes = TwDefineEnum("ToneMappingType", TonemmappingEnums, 2);

		TwSetParam(bar, nullptr, "size", TW_PARAM_INT32, 2, barSize);
		TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &uiRotate_, "opened=true axisz=-z group=object ");
		TwAddVarRW(bar, "specular type", specularTypes, &uiSpecularType_, " group='render' ");
		TwAddVarRW(bar, "irridiance type", irrianceTypes, &uiIrridianceType_, " group='render'");
		TwAddVarRW(bar, "tonemapping type", tonemappingTypes, &uiTonemappingType_, " group='render'");
		TwAddVarRW(bar, "output color", outputTypes, &uiOutputType_, " group='render'");
		TwAddVarRW(bar, "exposoure", TW_TYPE_FLOAT, &uiExpoure_, "min=0.1 max=20.0 step=0.1 group=render");

		//uiExpoure_
		//camara
		pCamara_ = this->GetSceneMgr().GetMainCamara();
		pCamara_->LookAt(XMFLOAT4(0, 0, -100, 0), XMFLOAT4(0, 0, 0, 0));
		camaraController_.SetCamara(pCamara_);

		//model
		pistol_ = mgr->CreateModelFromFile("pbr_model/pistol/pistol.fbx");
		Null_Return_False((perframeBuffer_ = mgr->CreateConstantBuffer(sizeof(CB_PerFrame))));

		//sample
		Null_Return_False((samplers_[0] = mgr->CreateLinearSamplerState()));
		samplers_[1] = samplers_[2] = samplers_[0];

		
		pointSampler_ = mgr->CreatePointSamplerState();
		return true;
	};

	virtual void WindowResize(int width, int height,
		ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		aspect = (float)width / (float)height;
		pCamara_->SetProject(BaseCamara::eCamara_Perspective, XM_PI / 4, aspect, 0.1f, 1000.0f);
	};


	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		context->PSSetSamplers(0, 3, samplers_);

		XMVECTOR rotQuaternion = XMLoadFloat4(&uiRotate_);
		XMMATRIX rotMatrix = XMMatrixRotationQuaternion(rotQuaternion);

		XMMATRIX viewProject = pCamara_->GetViewProjectMatrix();
		XMMATRIX worldViewProject = XMMatrixMultiplyTranspose(rotMatrix, viewProject);
		XMMATRIX worldStoreType = XMMatrixTranspose(rotMatrix);

		CB_PerFrame perFrame;
		XMStoreFloat4x4(&perFrame.world, worldStoreType);
		XMStoreFloat4x4(&perFrame.worldViewProject, worldViewProject);
		perFrame.camaraPos = pCamara_->GetEyePosStoreType();
		perFrame.specularType = uiSpecularType_;
		perFrame.irridianceType = uiIrridianceType_;
		perFrame.outputType = uiOutputType_;
		//Log_Info("specular type %d exposure %f frash rate:%f", uiSpecularType_, uiExpoure_, this->GetFPS());
		
		ResourceMgr::GetSingletonPtr()->MappingBufferWriteOnly(perframeBuffer_, &perFrame, sizeof(CB_PerFrame));
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);
		ID3D11RenderTargetView* mainRT = this->GetMainRT();
		ID3D11DepthStencilView* mainDSV = this->GetMainDSV();

		pistol_->SetConstBuffer(perframeBuffer_);
		pistol_->Render(context);



		//ui
		TwDraw();

		ID3D11ShaderResourceView*    pSRV[eShaderResource_Irridiance] = { 0, 0, 0, 0};
		context->PSSetShaderResources(0, eShaderResource_Irridiance, pSRV);
	};

	virtual int MsgProcess(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		// Send event message to AntTweakBar
		if (TwEventWin(hwnd, msg, wparam, lparam))
		{
			return 0;
		}
		camaraController_.ProcessMessage(hwnd, msg, wparam, lparam);
	}

	virtual void UpdateScene(float elapse)
	{
		camaraController_.Update(elapse);
	}

	virtual void Exit()
	{
		TwTerminate();
	};
private:
	ID3D11Buffer*         perframeBuffer_;
	ID3D11SamplerState*   samplers_[3];
	ID3D11SamplerState*   pointSampler_;

	HdrPresentProcess*    hdrProcess_;
	PostProcessChain      postProcessChain_;

	BaseModel*            pistol_;
	SkyBox                skybox_;
	FirstPersonController camaraController_;
	BaseCamara*           pCamara_;

	float				  aspect;
	XMFLOAT4              uiRotate_;
	unsigned              uiSpecularType_;
	unsigned              uiIrridianceType_;
	unsigned              uiTonemappingType_;
	unsigned              uiOutputType_;
	float                 uiExpoure_;
};




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR pScmdline, int iCmdshow)
{
	//ul::SetBreakPointAtMemoryLeak(46691);
	ul::OpenConsoleAndDebugLeak();
	Lession1_Frame  app;

	if (app.Initialize(1024, 768))
	{
		app.Run();
	}
	app.Shutdown();

	return 0;
}

