#include<Windows.h>
#include<memory>

#include<xnamath.h>

#include"util/tools.h"
#include"Application.h"
#include"res_mgr.h"
#include"model_render.h"
#include"d3d11_helper.h"

using namespace ul;

struct UB_Coeffs
{
	XMFLOAT3 val[9];
	float sumWeight;
};


struct CB_ComputeResolution
{
	float Resolution[2];
	int   face;
	int   padding;
};


#define THREAD_DIMENSION 4
#define RESOLUTION 1024
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

		envTexture_ = mgr->CreateTextureFromFile("../res/uffizi_cross.dds");

		float r[9], g[9], b[9];
		
		ID3D11Resource* texture;
		D3D11_TEXTURE2D_DESC desc;
		envTexture_->GetResource(&texture);
		((ID3D11Texture2D*)texture)->GetDesc(&desc);
		ENV_MAP_WIDTH = desc.Width;
		D3DX11SHProjectCubeMap(context, 3, (ID3D11Texture2D*)texture, r, g, b);
		for (int i = 0; i < 9; ++i)
		{
			Log_Info("c.val[%d] = float3(%f, %f, %f);",i, r[i], g[i], b[i]);
		}

		// 环境贴图查找表
		D3D11_TEXTURE2D_DESC lukupDesc;
		lukupDesc.Width = ENV_MAP_WIDTH;
		lukupDesc.Height = ENV_MAP_WIDTH;
		lukupDesc.MipLevels = 1;
		lukupDesc.ArraySize = 1;
		lukupDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		lukupDesc.CPUAccessFlags = 0;
		lukupDesc.SampleDesc.Count = 1;
		lukupDesc.SampleDesc.Quality = 0;
		lukupDesc.Usage = D3D11_USAGE_DEFAULT;
		lukupDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		lukupDesc.MiscFlags = 0;
		Null_Return_Void(( processedTexture = mgr->CreateTexture2DNoData(lukupDesc) ));

		D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
		sbUAVDesc.Texture2D.MipSlice = 0;
		sbUAVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		sbUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		Null_Return_Void((processedUAV = mgr->CreateUnorderedAccessView(processedTexture, sbUAVDesc, ResourceMgr::eReleaseOnExit)));

		int ShBufferCounter = floor(std::log10((ENV_MAP_WIDTH / THREAD_DIMENSION)) / std::log10(2)) + 1;
		int BufferWidth = ENV_MAP_WIDTH;
		ShBufferVec.reserve(ShBufferCounter);
		ShUnorderedStructs.reserve(ShBufferCounter);

		D3D11_BUFFER_DESC unordered_buffer_desc;
		ZeroMemory(&unordered_buffer_desc, sizeof(unordered_buffer_desc));
		unordered_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		unordered_buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		unordered_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavbuffer_desc;
		ZeroMemory(&uavbuffer_desc, sizeof(uavbuffer_desc));
		uavbuffer_desc.Format = DXGI_FORMAT_UNKNOWN;
		uavbuffer_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavbuffer_desc.Buffer.FirstElement = 0;

		for (int i = 0; i < ShBufferCounter; ++i)
		{
			ID3D11Buffer *buffer;
			unordered_buffer_desc.StructureByteStride = sizeof(UB_Coeffs);
			unordered_buffer_desc.ByteWidth = sizeof(UB_Coeffs)* BufferWidth * BufferWidth;
			
			Null_Return_Void( 
				(buffer = mgr->CreateBuffer(unordered_buffer_desc, nullptr))
			);
			ShBufferVec.push_back(buffer);

			ID3D11UnorderedAccessView *uav;
			uavbuffer_desc.Buffer.NumElements = BufferWidth*BufferWidth;
			Null_Return_Void((uav = mgr->CreateUnorderedAccessView(buffer, uavbuffer_desc, ResourceMgr::eReleaseOnExit)));
			ShUnorderedStructs.push_back(uav);

			BufferWidth = BufferWidth / 2;
		}


		//cpu buffer
		int CpuBufferResolution = THREAD_DIMENSION;
		D3D11_BUFFER_DESC cpu_buffer_desc;
		ZeroMemory(&cpu_buffer_desc, sizeof(cpu_buffer_desc));
		cpu_buffer_desc.Usage = D3D11_USAGE_STAGING;
		cpu_buffer_desc.BindFlags = 0;
		cpu_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		cpu_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		cpu_buffer_desc.StructureByteStride = sizeof(UB_Coeffs);
		cpu_buffer_desc.ByteWidth = sizeof(UB_Coeffs)*CpuBufferResolution*CpuBufferResolution;

		//采样
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
		SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

		SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		Null_Return_Void((trilinear = mgr->CreateSamplerState(SamDesc)));  //三线性
		Null_Return_Void((ShCpuValue = mgr->CreateBuffer(cpu_buffer_desc, nullptr)));
		Null_Return_Void((SHvalueFirstPass = mgr->CreateComputeShader("main.hlsl", "FirstPass", "cs_5_0")));
		Null_Return_Void((SHvalueNextPass =  mgr->CreateComputeShader("main.hlsl", "NextPass", "cs_5_0")));
		Null_Return_Void((cs_testPass = mgr->CreateComputeShader("cs_test.hlsl", "copy", "cs_5_0")));
		
		resolutionBuffer_ = mgr->CreateConstantBuffer(sizeof(CB_ComputeResolution));
	};


	void TestComputerShader(ID3D11DeviceContext* context)
	{
		context->CSSetSamplers(0, 1, &trilinear);
		context->CSSetShaderResources(0, 1, &envTexture_);
		context->CSSetUnorderedAccessViews(0, 1, &processedUAV, nullptr);
		context->CSSetConstantBuffers(0, 1, &resolutionBuffer_);
		context->CSSetShader(cs_testPass, nullptr, 0);
		context->Dispatch(ENV_MAP_WIDTH / THREAD_DIMENSION, ENV_MAP_WIDTH / THREAD_DIMENSION, 1);
	}

	void ShValueFirstCompute(ID3D11DeviceContext* context, int dispatchDimension, int curResolution, int face)
	{

		ShOutValueUAV = ShUnorderedStructs[0];
		context->CSSetSamplers(0, 1, &trilinear);
		context->CSSetShaderResources(0, 1, &envTexture_);
		context->CSSetUnorderedAccessViews(1, 1, &ShOutValueUAV, nullptr);

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		memset(&MappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		Fail_Return_Void(context->Map(resolutionBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		CB_ComputeResolution* pConstants = (CB_ComputeResolution*)MappedResource.pData;
		pConstants->Resolution[0] = curResolution;
		pConstants->Resolution[1] = curResolution;
		pConstants->face = face;
		context->Unmap(resolutionBuffer_, 0);

		context->CSSetConstantBuffers(0, 1, &resolutionBuffer_);
		context->CSSetShader(SHvalueFirstPass, nullptr, 0);
		context->Dispatch(dispatchDimension, dispatchDimension, 1);
	}

	void ShValueNextCompute(ID3D11DeviceContext* context, int lev, int dispatchDimension, int curResolution, int face)
	{
		ShInValueUAV = ShUnorderedStructs[lev];
		ShOutValueUAV = ShUnorderedStructs[lev+1];
		ID3D11UnorderedAccessView* views[2] = { ShInValueUAV, ShOutValueUAV };
		context->CSSetUnorderedAccessViews(0, 2, views, nullptr);
		context->CSSetConstantBuffers(0, 1, &resolutionBuffer_);

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		memset(&MappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		Fail_Return_Void(context->Map(resolutionBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		CB_ComputeResolution* pConstants = (CB_ComputeResolution*)MappedResource.pData;
		pConstants->Resolution[0] = curResolution;
		pConstants->Resolution[1] = curResolution;
		pConstants->face = face;
		context->Unmap(resolutionBuffer_, 0);

		context->CSSetShader(SHvalueNextPass, nullptr, 0);
		context->Dispatch(dispatchDimension, dispatchDimension, 1);
	}

	void SetParameter(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		memset(&MappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		Fail_Return_Void(context->Map(resolutionBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		CB_ComputeResolution* pConstants = (CB_ComputeResolution*)MappedResource.pData;
		pConstants->Resolution[0] = ENV_MAP_WIDTH;
		pConstants->Resolution[1] = ENV_MAP_WIDTH;
		pConstants->face =1;
		context->Unmap(resolutionBuffer_, 0);
	}

	virtual void RenderFrame(ID3D11Device *dev,
		ID3D11DeviceContext* context)
	{
		this->SetParameter(dev, context);


		if (!CS_test)
		{
			TestComputerShader(context);

			if (FAILED(D3DX11SaveTextureToFile(context, processedTexture, D3DX11_IFF_DDS, "5.dds")))
			{
				printf("save texture  error .\n");
			}
			else{
				CS_test = true;
			}
		}

		if (!ShCached)
		{
			UB_Coeffs finalResult;
			memset(&finalResult, 0, sizeof(finalResult));
			XMVECTOR result[9];
			XMVECTOR val[9];
			float resultWeight = 0.0f;

			for (ulUint n = 0; n < 9; ++n)
			{
				result[n] = XMLoadFloat4(&XMFLOAT4(0, 0, 0, 0));
			}

			//for each face
			for (int face = 0; face < 6; ++face)
			{
				int DispatchGroup = ENV_MAP_WIDTH / THREAD_DIMENSION;
				int CurTextureDimension = ENV_MAP_WIDTH;

				ShValueFirstCompute(context, DispatchGroup, CurTextureDimension, face);

				CurTextureDimension /= 2;
				DispatchGroup /= 2;
				for (int mip = 0; mip < ShUnorderedStructs.size() - 1; ++mip)
				{
					ShValueNextCompute(context, mip, DispatchGroup, CurTextureDimension, face);
					DispatchGroup = DispatchGroup / 2;
					CurTextureDimension /= 2;
				}
				context->CopyResource(ShCpuValue, ShBufferVec[ShBufferVec.size()-1]);

				D3D11_MAPPED_SUBRESOURCE MappedResource;
				Fail_Return_Void(context->Map(ShCpuValue, 0, D3D11_MAP_READ, 0, &MappedResource));
				UB_Coeffs* pConstants = (UB_Coeffs*)MappedResource.pData;

				float faceWeight = 0;
				for (int dem = 0; dem < THREAD_DIMENSION*THREAD_DIMENSION; ++dem)
				{
					UB_Coeffs* coeff = &pConstants[dem];
					for (ulUint i = 0; i < 9; ++i)
					{
						val[i] = XMLoadFloat3(&coeff->val[i]);
						result[i] += val[i];
					}
					faceWeight += coeff->sumWeight;
					resultWeight += coeff->sumWeight;
				}

				context->Unmap(ShCpuValue, 0);
			}

			for (ulUint n = 0; n < 9; ++n)
			{
				result[n] *= 4 * XM_PI / resultWeight;
				XMStoreFloat3(&finalResult.val[n], result[n]);
			}
			finalResult.sumWeight = resultWeight;
			for (int i = 0; i < 9; ++i)
			{
				Log_Info("c.val[%d] = float3(%f, %f, %f);", i,finalResult.val[i].x, finalResult.val[i].y, finalResult.val[i].z);
			}
			ShCached = true;
		}

	};

	
	virtual void MsgProcess (
		HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam) override
	{

	}

	virtual void Exit() override
	{

	}

	virtual void UpdateScene(float elapse)override
	{

	}
private:
	ID3D11Buffer*						  resolutionBuffer_;
	ID3D11Buffer*						  ShCpuValue = nullptr;
	ID3D11ShaderResourceView			  *envTexture_;
	ID3D11Texture2D                       *processedTexture;
	ID3D11UnorderedAccessView             *processedUAV;

	ID3D11SamplerState *trilinear;
	ID3D11ComputeShader					  *cs_testPass = nullptr;
	ID3D11ComputeShader					  *SHvalueFirstPass = nullptr;
	ID3D11ComputeShader					  *SHvalueNextPass = nullptr;
	ID3D11ShaderResourceView			  *SHConvolutionSrcSRV = nullptr;
	ID3D11UnorderedAccessView			  *ShInValueUAV = nullptr;
	ID3D11UnorderedAccessView			  *ShOutValueUAV = nullptr;
	vector<ID3D11UnorderedAccessView*>    ShUnorderedStructs;
	vector<ID3D11Buffer*>			      ShBufferVec;
	int                                   ENV_MAP_WIDTH;
	bool ShCached = false;
	bool CS_test = false;
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