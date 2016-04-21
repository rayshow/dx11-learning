//资源管理器


#ifndef RESOURSE_MANAGER_H__
#define RESOURSE_MANAGER_H__

#include <d3d11.h>
#include <D3DX11tex.h>
#include <D3DX11core.h>
#include <D3DX11async.h>
#include <d3dcompiler.h>

#include <vector>
#include <assert.h>
#include <hash_map>
#include <map>
#include <memory>
#include <stdio.h>

#pragma comment(lib, "d3dcompiler.lib")

#if defined (DEBUG)||(_DEBUG) 
		#pragma comment(lib, "d3dx11d.lib")
#else
		#pragma comment(lib, "d3dx11.lib")	
#endif

#include"../design_frame/singleton.h"
#include"../util/UlHelper.h"
#include"../render/MeshRender.h"
#include"CommonModelLoader.h"


using std::vector;
using std::string;
using std::wstring;
using std::map;
using std::unique_ptr;

namespace ul
{
	//view resource 
	typedef vector<ID3D11ShaderResourceView*>             ShaderResViewVector;
	typedef vector<ID3D11RenderTargetView*>               RenderTargetViewVector;
	typedef vector<ID3D11DepthStencilView*>               DepthStencilViewVector;
	typedef vector<ID3D11UnorderedAccessView*>            UnorderedAccessViewVector;
	typedef vector<ID3D11Texture2D*>                      Texture2DVector;

	//shader resource
	typedef vector<ID3D11VertexShader*>                   VertexShaderVector;
	typedef vector<ID3D11PixelShader*>                    PixelShaderVector;
	typedef vector<ID3D11GeometryShader*>                 GeometryShaderVector;
	typedef vector<ID3D11ComputeShader*>                  ComputeShaderVector;

	//state resource
	typedef vector<ID3D11SamplerState*>                   SamplerStateVector;
	typedef vector<ID3D11BlendState*>                     BlendStateVector;
	typedef vector<ID3D11InputLayout*>                    InputLayoutVector;
	typedef vector<ID3D11RasterizerState*>                RasterVector;
	typedef vector<ID3D11Buffer*>                         BufferVector;

	//duplicable use resource
	typedef map< string, ID3D11ShaderResourceView*>       ShaderResourceFilePool;
	typedef map< string, BaseModel*>					  ModelPool;
	typedef map< string, StaticMeshRender*>               StaticMeshRenderPool;
	typedef map< string, ID3DX11Effect*>                  EffectPool;

	//view iter
	typedef ShaderResViewVector::iterator				  ShaderResViewVectorIter;
	typedef RenderTargetViewVector::iterator			  RenderTargetViewVectorIter;
	typedef DepthStencilViewVector::iterator			  DepthStencilViewVectorIter;
	typedef UnorderedAccessViewVector::iterator			  UnorderedAccessViewIter;
	typedef Texture2DVector::iterator					  Texture2DVectorIter;

	//shader iter
	typedef VertexShaderVector::iterator				  VertexShaderVectorIter;
	typedef PixelShaderVector::iterator					  PixelShaderVectorIter;
	typedef GeometryShaderVector::iterator				  GeometryShaderVectorIter;
	typedef ComputeShaderVector::iterator				  ComputeShaderVectorIter;

	//state iter
	typedef SamplerStateVector::iterator				  SamplerStateVectorIter;
	typedef BlendStateVector::iterator					  BlendStateVectorIter;
	typedef InputLayoutVector::iterator					  InputLayoutVectorIter;
	typedef BufferVector::iterator						  BufferVectorIter;
	typedef RasterVector::iterator						  RasterVectorIter;
	typedef ShaderResourceFilePool::iterator			  TexturePoolIter;
	typedef ModelPool::iterator							  ModelPoolIter;
	typedef StaticMeshRenderPool::iterator			      StaticMeshRenderPoolIter;
	typedef EffectPool::iterator                          EffectPoolIter;

	//窗口重置后需要回收
	struct SReleaseOnWindowReize
	{
		ShaderResViewVector			shaderResViews;
		RenderTargetViewVector		renderTargetViews;
		DepthStencilViewVector		depthStencilViews;
		Texture2DVector				texture2Ds;
		UnorderedAccessViewVector   unorderViews;
	};

	//退出回收
	struct SReleaseOnExit
	{
		//view resource
		ShaderResViewVector		  shaderResViews;
		RenderTargetViewVector	  renderTargetViews;
		DepthStencilViewVector	  depthStencilViews;
		UnorderedAccessViewVector unorderViews;
		Texture2DVector			  texture2Ds;

		//shader resource
		VertexShaderVector		  vertexShaders;
		PixelShaderVector		  pixelShaders;
		GeometryShaderVector	  geometryShaders;
		ComputeShaderVector		  computeShaders;

		//state resource
		SamplerStateVector		  samplerStates;
		BlendStateVector		  blendStates;
		InputLayoutVector		  inputLayouts;
		BufferVector			  buffers;
		RasterVector			  rasterStates;

		//pool
		ShaderResourceFilePool	  texturePool;
		ModelPool				  modelPool;
		StaticMeshRenderPool	  staticRenderPool;
		EffectPool                effectPool;
	};

	//类型
	enum EReleaseType
	{
		eRelease_OnResize,
		eRelease_OnExit
	};

	static bool CompileShaderFromFile(
		const char* szFileName,
		const char* szEntryPoint,
		const char* szShaderModel,
		ID3DBlob**  ppBlobOut)
	{
		if (szFileName[0] == '\0' || szEntryPoint[0] == '\0' || szShaderModel[0] == '\0')
		{
			Log_Err("compile parameter error");
			return false;
		}
		DWORD dwShaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
		dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		AutoReleasePtr<ID3DBlob> ErrorBlobPtr;
		HRESULT hr = D3DX11CompileFromFileA(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, nullptr, ppBlobOut, ErrorBlobPtr.GetPtr(), nullptr);
		if (Fail(hr))
		{
			if (hr == D3D11_ERROR_FILE_NOT_FOUND)
			{
				Log_Err("not found file:%s", szFileName);
				return false;
			}
			Log_Err("error compile:%s", ErrorBlobPtr->GetBufferPointer());
			return false;
		}
		return true;
	}



	//回收集
	class ResourceMgr : public Singleton<ResourceMgr>
	{
	public:
		
	private:
		SReleaseOnWindowReize		   releaseSetOnResize_;
		SReleaseOnExit                 releaseSetOnExit_;
		string                         resourceBasePath_;
		ID3D11Device*                  pDevice_;
		ID3D11DeviceContext*           pContext_;
		
	public:
		ResourceMgr(){}
	private:

		//创建顶点着色器
		inline ID3D11VertexShader*
		createVertexShader(ID3DBlob * blob)
		{
			ID3D11VertexShader *pVertexShader;
			Fail_Return_Null(pDevice_->CreateVertexShader(
				blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pVertexShader));
			releaseSetOnExit_.vertexShaders.push_back(pVertexShader);
			return pVertexShader;
		}

		//创建inputLayout
		inline ID3D11InputLayout*
		createInputLayout(const D3D11_INPUT_ELEMENT_DESC *descs, ulUint count, ID3DBlob* pBlob)
		{
			ID3D11InputLayout *pLayout;
			Fail_Return_Null( pDevice_->CreateInputLayout(descs, count,
				pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pLayout));
			releaseSetOnExit_.inputLayouts.push_back(pLayout);
			return pLayout;
		}

		//创建片元着色器
		inline ID3D11PixelShader* createPixelShader(ID3DBlob * blob)
		{
			ID3D11PixelShader *pPixelShader;
			Fail_Return_Null(pDevice_->CreatePixelShader(blob->GetBufferPointer(),
				blob->GetBufferSize(), nullptr, &pPixelShader));
			releaseSetOnExit_.pixelShaders.push_back(pPixelShader);
			return pPixelShader;
		}

		//创建计算着色器
		inline ID3D11ComputeShader* createComputeShader(ID3DBlob * blob)
		{
			ID3D11ComputeShader *pComputerShader;
			Fail_Return_Null(pDevice_->CreateComputeShader(blob->GetBufferPointer(),
				blob->GetBufferSize(), nullptr, &pComputerShader));
			releaseSetOnExit_.computeShaders.push_back(pComputerShader);
			return pComputerShader;
		}

		//创建几何着色器
		inline ID3D11GeometryShader*
		createGeometryShader(ID3DBlob * blob)
		{
			ID3D11GeometryShader *pGeomShader;
			Fail_Return_Null(pDevice_->CreateGeometryShader(blob->GetBufferPointer()
				, blob->GetBufferSize(), nullptr, &pGeomShader));
			releaseSetOnExit_.geometryShaders.push_back(pGeomShader);
			return pGeomShader;
		}

	public:
		//初始化
		inline void
		init(ID3D11Device* dev, ID3D11DeviceContext* context){
			pDevice_  = dev;
			pContext_ = context;
			Log_Info("resource manager initialized.");
		}

		inline string GetResourceBasePath() const { return resourceBasePath_; }

		inline void SetResourceBasePath(const string& resourcePath)
		{
			bool endWithSeperate = false;
			if (resourcePath.find_last_of("/") == resourcePath.length() - 1)
			{
				endWithSeperate = true;
			}

			resourceBasePath_ = resourcePath;
			if (!endWithSeperate)
				resourceBasePath_ = resourceBasePath_ + "/";
		}

		//mapping constant buffer
		inline bool MappingBufferWriteOnly(ID3D11Buffer* constBuffer,
			void* buffer, ulUint bufferSize)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			memset(&mappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
			Fail_Return_False(pContext_->Map(constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			memcpy(mappedResource.pData, buffer, bufferSize);
			pContext_->Unmap(constBuffer, 0);
			return true;
		}

		//rasterize state
		inline ID3D11RasterizerState*
		CreateRasterState(D3D11_RASTERIZER_DESC& desc)
		{
			ID3D11RasterizerState *rasterState;
			Fail_Return_Null(pDevice_->CreateRasterizerState(&desc, &rasterState));
			releaseSetOnExit_.rasterStates.push_back(rasterState);
			return rasterState;
		}

		//srv
		inline ID3D11ShaderResourceView*
		CreateShaderResourceView(
			ID3D11Resource* from,
			D3D11_SHADER_RESOURCE_VIEW_DESC* desc,
			EReleaseType em = eRelease_OnExit)
		{
			ID3D11ShaderResourceView *pSrv;
			Fail_Return_Null(pDevice_->CreateShaderResourceView(from, desc, &pSrv));
			switch (em)
			{
				case eRelease_OnResize:
					releaseSetOnResize_.shaderResViews.push_back(pSrv);
					break;
				case eRelease_OnExit:
					releaseSetOnExit_.shaderResViews.push_back(pSrv);
					break;
				default:
					assert(0);
			}
			return pSrv;
		}


		//创建渲染视图
		inline ID3D11RenderTargetView*
		CreateRenderTargetView(
			ID3D11Resource* from,
			D3D11_RENDER_TARGET_VIEW_DESC* desc,
			EReleaseType em = eRelease_OnExit)
		{
			ID3D11RenderTargetView *pRtv;
			Fail_Return_Null(pDevice_->CreateRenderTargetView(from, desc, &pRtv));

			switch (em)
			{
			case eRelease_OnResize:
				releaseSetOnResize_.renderTargetViews.push_back(pRtv);
				break;
			case eRelease_OnExit:
				releaseSetOnExit_.renderTargetViews.push_back(pRtv);
				break;
			default:
				assert(0);
			}
			return pRtv;
		}

		//创建纹理
		inline ID3D11ShaderResourceView*
		CreateTextureFromFile(const string& fileName)
		{
			ID3D11ShaderResourceView *pSrv;
			TexturePoolIter find = releaseSetOnExit_.texturePool.find(fileName);
			//exists
			if (find != releaseSetOnExit_.texturePool.end())
			{
				return find->second;
			}
			Fail_Return_Null_With_Msg(
				D3DX11CreateShaderResourceViewFromFileA(pDevice_, fileName.c_str(), nullptr, nullptr, &pSrv, nullptr),
				"can't find file %s or format not support.", fileName.c_str());
			releaseSetOnExit_.texturePool.insert(std::make_pair(fileName, pSrv));
			Log_Info("read texture %s ok", fileName.c_str());
			return pSrv;
		}

		//创建模型
		inline BaseModel* CreateModelFromFile(const string& fileName)
		{
			ModelPoolIter find = releaseSetOnExit_.modelPool.find(fileName);
			//exists
			if (find != releaseSetOnExit_.modelPool.end())
			{
				return find->second;
			}
			Log_Info("loading model %s ", fileName.c_str());

			CommonModelLoader loader;
			SModelData data;
		
			False_Return_Null_With_Msg(
				loader.LoadFile(resourceBasePath_,  fileName, data),
				"loading model %s error.", fileName.c_str());
			BaseModel* model = new BaseModel();
			Null_Return_Null(model);
			model->Create(pDevice_, data);
			ModelData_Free(data);

			releaseSetOnExit_.modelPool.insert(std::make_pair(fileName, model));
			Log_Info("read model %s ok", fileName.c_str());
			return model;
		}

		inline StaticMeshRender* CreateStaticMeshRenderFromFile(const string& fileName)
		{
			StaticMeshRenderPoolIter find = releaseSetOnExit_.staticRenderPool.find(fileName);
			//exists
			if (find != releaseSetOnExit_.staticRenderPool.end())
			{
				return find->second;
			}

			Log_Info("loading model %s ", fileName.c_str());
			CommonModelLoader loader;
			SModelData data;
			False_Return_Null_With_Msg(
				loader.LoadFile(resourceBasePath_, fileName, data),
				"loading model %s error.", fileName.c_str()
			);

			StaticMeshRender* model = new StaticMeshRender();
			Null_Return_Null(model);
			model->Create(pDevice_, data);
			ModelData_Free(data);

			releaseSetOnExit_.staticRenderPool.insert(std::make_pair(fileName, model));
			Log_Info("read model %s ok", fileName.c_str());
			return model;
		}

		//深度缓冲视图
		inline ID3D11DepthStencilView*
		CreateDepthStencilView(
			ID3D11Resource* from,
			D3D11_DEPTH_STENCIL_VIEW_DESC* desc,
			EReleaseType em = eRelease_OnExit)
		{
			ID3D11DepthStencilView *pDsv;
			Fail_Return_Null(pDevice_->CreateDepthStencilView(from, desc, &pDsv));

			switch (em)
			{
			case eRelease_OnResize:
				releaseSetOnResize_.depthStencilViews.push_back(pDsv);
				break;
			case eRelease_OnExit:
				releaseSetOnExit_.depthStencilViews.push_back(pDsv);
				break;
			default:
				assert(0);
			}
			return pDsv;
		}

		//无序视图
		inline ID3D11UnorderedAccessView*
		CreateUnorderedAccessView(
			ID3D11Resource* from,
			const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc,
			EReleaseType em = eRelease_OnExit)
		{
			ID3D11UnorderedAccessView *pUav;
			Fail_Return_Null(pDevice_->CreateUnorderedAccessView(from, &desc, &pUav));
			switch (em)
			{
			case eRelease_OnResize:
				releaseSetOnResize_.unorderViews.push_back(pUav);
				break;
			case eRelease_OnExit:
				releaseSetOnExit_.unorderViews.push_back(pUav);
				break;
			default:
				assert(0);
			}
			return pUav;
		}

		inline ID3D11Texture2D*
		CreateTexture2DNoData(
			const D3D11_TEXTURE2D_DESC& desc,
			EReleaseType em = eRelease_OnExit)
		{
			ID3D11Texture2D *pTexture2D;
			pDevice_->CreateTexture2D(&desc, nullptr, &pTexture2D);
			switch (em)
			{
			case eRelease_OnResize:
				releaseSetOnResize_.texture2Ds.push_back(pTexture2D);
				break;
			case eRelease_OnExit:
				releaseSetOnExit_.texture2Ds.push_back(pTexture2D);
				break;
			default:
				assert(0);
			}
			return pTexture2D;
		}


		//编译和创建顶点着色器
		inline ID3D11VertexShader*
		CreateVertexShader(
			const char* fileName,
			const char* functionName,
			const char* v)
		{
			ID3D11VertexShader *pVertexShader = nullptr;
			AutoReleasePtr<ID3DBlob> blobPtr;
			False_Return_Null_With_Msg(
				CompileShaderFromFile(fileName, functionName, v, blobPtr.GetPtr()),
				"compile vertex shader %s, %s error.", fileName, functionName
			);
			Null_Return_Null_With_Msg(
				(pVertexShader = this->createVertexShader(blobPtr.Get())),
				"compile  vertex shader %s function %s error", fileName, functionName
			);
			return pVertexShader;
		}

		//编译和创建顶点着色器
		inline ID3D11VertexShader*
		CreateVertexShaderResBase(
			const char* fileName,
			const char* functionName,
			const char* v)
		{
			string fullFileName = resourceBasePath_ + fileName;
			return this->CreateVertexShader(fullFileName.c_str(), functionName, v);
		}


		//编译和创建有顶点格式的顶点着色器
		inline ID3D11VertexShader*
		CreateVertexShaderAndInputLayout(
			const char* fileName,
			const char* functionName,
			const char* v,
			const D3D11_INPUT_ELEMENT_DESC* loDesc,
			int descSize,
			ID3D11InputLayout** ppLayout)
		{
			ID3D11VertexShader *pVertexShader = nullptr;
			AutoReleasePtr<ID3DBlob> blobPtr;
			False_Return_Null_With_Msg(
				CompileShaderFromFile(fileName, functionName, v, blobPtr.GetPtr()),
				"compile shader file %s ,function %s error", fileName, functionName
			);
			Null_Return_Null_With_Msg(
				(pVertexShader = this->createVertexShader(blobPtr.Get())),
				"create shader file %s ,function %s error", fileName, functionName
			);
			Null_Return_Null_With_Msg(
				(*ppLayout = this->createInputLayout(loDesc, descSize, blobPtr.Get())),
				"create shader file %s ,function %s vertex layout error", fileName, functionName
			);
			return pVertexShader;
		}

		//编译和创建有顶点格式的顶点着色器
		inline ID3D11VertexShader*
		CreateVertexShaderAndInputlayoutResBase(
			const char* fileName,
			const char* functionName,
			const char* v,
			const D3D11_INPUT_ELEMENT_DESC* loDesc,
			int descSize,
			ID3D11InputLayout** layout)
		{
			string fullFileName = resourceBasePath_ + fileName;
			return this->CreateVertexShaderAndInputLayout(fullFileName.c_str(),
				functionName, v, loDesc, descSize, layout);
		}

		inline ID3D11PixelShader*
		CreatePixelShader(
			const char* fileName,
			const char* functionName,
			const char* v)
		{
			ID3D11PixelShader *ps = nullptr;
			AutoReleasePtr<ID3DBlob> blobPtr;
			False_Return_Null_With_Msg(
				CompileShaderFromFile(fileName, functionName, v, blobPtr.GetPtr()),
				"compile  shader file %s ,pixel function %s error", fileName, functionName
			);
			Null_Return_Null_With_Msg(
				((ps = this->createPixelShader(blobPtr.Get()))),
				"create  shader file %s ,pixel function %s error", fileName, functionName
			);
			return ps;
		}

		inline ID3D11PixelShader*
		CreatePixelShaderResBase(
			const char* fileName,
			const char* functionName,
			const char* v)
		{
			string fullPathName = resourceBasePath_ + fileName;
			return this->CreatePixelShader(fullPathName.c_str(), functionName, v);
		}

		inline ID3D11GeometryShader*
		CreateGeometryShader(
			char* fileName,
			char* functionName,
			char* v)
		{
			ID3D11GeometryShader *gs = nullptr;
			AutoReleasePtr<ID3DBlob> blobPtr;
			False_Return_Null_With_Msg(
				CompileShaderFromFile(fileName, functionName, v, blobPtr.GetPtr()),
				"compile  shader file %s ,geometry function %s error", fileName, functionName
			);
			Null_Return_Null_With_Msg(
				((gs = this->createGeometryShader(blobPtr.Get()))),
				"create  shader file %s ,geometry function %s error", fileName, functionName
			);
			return gs;
		}


		inline ID3D11ComputeShader*
		CreateComputeShader(
			const char* fileName,
			const char* functionName,
			const char* v)
		{
			ID3D11ComputeShader *cs = nullptr;
			AutoReleasePtr<ID3DBlob> blobPtr;
			False_Return_Null_With_Msg(
				CompileShaderFromFile(fileName, functionName, v, blobPtr.GetPtr()),
				"compile  shader file %s ,compute function %s error", fileName, functionName
			);
			Null_Return_Null_With_Msg(
				((cs = this->createComputeShader(blobPtr.Get()))),
				"create  shader file %s ,compute function %s error", fileName, functionName
			);
			return cs;
		}



		//创建采样状态
		inline ID3D11SamplerState*
		CreateSamplerState(D3D11_SAMPLER_DESC &desc)
		{
			ID3D11SamplerState *samplerState;
			Fail_Return_Null(pDevice_->CreateSamplerState(&desc, &samplerState));
			releaseSetOnExit_.samplerStates.push_back(samplerState);
			return samplerState;
		}

		inline ID3D11SamplerState* CreateNormalSampler(D3D11_FILTER filter, 
			D3D11_TEXTURE_ADDRESS_MODE addressMode)
		{
			D3D11_SAMPLER_DESC SamDesc;
			SamDesc.Filter =   filter;
			SamDesc.AddressU = addressMode;
			SamDesc.AddressV = addressMode;
			SamDesc.AddressW = addressMode;
			SamDesc.MipLODBias = 0.0f;
			SamDesc.MaxAnisotropy = 16;
			SamDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
			SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 1.0;
			SamDesc.MinLOD = 0;
			SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
			return this->CreateSamplerState(SamDesc);
		}

		//三线性采样
		inline ID3D11SamplerState*
			CreateTrilinearSamplerState()
		{
			return this->CreateNormalSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
		}

		//向异性采样
		inline ID3D11SamplerState*
			CreateAnisotropicSamplerState()
		{
			return this->CreateNormalSampler(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP);
		}

		//点采样
		inline ID3D11SamplerState*
			CreatePointSamplerState()
		{
			return this->CreateNormalSampler(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_BORDER);
		}

		//线性采样
		inline ID3D11SamplerState* 
			CreateLinearSamplerState()
		{
			return this->CreateNormalSampler(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP);
		}

		//创建混合状态
		inline ID3D11BlendState*
		CreateBlendState(D3D11_BLEND_DESC &desc)
		{
			ID3D11BlendState *blendState;
			Fail_Return_Null(pDevice_->CreateBlendState(&desc, &blendState));
			releaseSetOnExit_.blendStates.push_back(blendState);
			return blendState;
		}

		//创建缓存
		inline ID3D11Buffer*
		CreateBuffer(D3D11_BUFFER_DESC& desc, const D3D11_SUBRESOURCE_DATA *data)
		{
			ID3D11Buffer *buffer;
			Fail_Return_Null(pDevice_->CreateBuffer(&desc, data, &buffer));
			releaseSetOnExit_.buffers.push_back(buffer);
			return buffer;
		}

		inline ID3D11Buffer*
		CreateVertexBuffer(void *memory, ulUint bytewide)
		{
				ID3D11Buffer* vb;
				D3D11_BUFFER_DESC vbd;
				D3D11_SUBRESOURCE_DATA vdata;

				vbd.Usage = D3D11_USAGE_IMMUTABLE;
				vbd.ByteWidth = bytewide;
				vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				vbd.CPUAccessFlags = 0;
				vbd.MiscFlags = 0;
				vbd.StructureByteStride = 0;

				ZeroMemory(&vdata, sizeof(vdata));
				vdata.pSysMem = memory;

				Null_Return_Null_With_Msg((vb = this->CreateBuffer(vbd, &vdata)));
				return vb;
		}

		inline ID3D11Buffer*
			CreateIndiceBuffer(void *memory, ulUint bytewide)
		{
				ID3D11Buffer *ib;
				D3D11_BUFFER_DESC ibd;
				D3D11_SUBRESOURCE_DATA  idata;

				ibd.Usage = D3D11_USAGE_IMMUTABLE;
				ibd.ByteWidth = bytewide;
				ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				ibd.CPUAccessFlags = 0;
				ibd.MiscFlags = 0;
				ibd.StructureByteStride = 0;

				ZeroMemory(&idata, sizeof(idata));
				idata.pSysMem = memory;

				Null_Return_Null_With_Msg(
					(ib = this->CreateBuffer(ibd, &idata)),
					"create indice buffer error."
					);
				return ib;

		}

		inline ID3D11Buffer*
			CreateConstantBuffer(ulUint bytewide)
		{
			ID3D11Buffer *cb;
			D3D11_BUFFER_DESC Desc;
			Desc.Usage = D3D11_USAGE_DYNAMIC;
			Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			Desc.MiscFlags = 0;
			Desc.ByteWidth = bytewide;
			Null_Return_Null_With_Msg(
				(cb = this->CreateBuffer(Desc, nullptr)),
				"create constant buffer error."
			);
			return cb;
		}



		//窗口重置时释放资源
		inline void ReleaseLoadedResourcePerSwapChain()
		{
			//release SRV
			SAFE_RELEASE_VECTOR<ShaderResViewVector>(releaseSetOnResize_.shaderResViews);

			//release RT
			SAFE_RELEASE_VECTOR<RenderTargetViewVector>(releaseSetOnResize_.renderTargetViews);

			//release DSV
			SAFE_RELEASE_VECTOR<DepthStencilViewVector>(releaseSetOnResize_.depthStencilViews);

			//release texuture2d
			SAFE_RELEASE_VECTOR<Texture2DVector>(releaseSetOnResize_.texture2Ds);
		}

		//app退出时释放
		inline void ReleaseLoadedResourceOnExit()
		{
			//release SRV
			SAFE_RELEASE_VECTOR<ShaderResViewVector>(releaseSetOnExit_.shaderResViews);

			//release RT
			SAFE_RELEASE_VECTOR<RenderTargetViewVector>(releaseSetOnExit_.renderTargetViews);

			//release DSV
			SAFE_RELEASE_VECTOR<DepthStencilViewVector>(releaseSetOnExit_.depthStencilViews);

			//release UAV
			SAFE_RELEASE_VECTOR<UnorderedAccessViewVector>(releaseSetOnExit_.unorderedViews);

			//release texuture2d
			SAFE_RELEASE_VECTOR<Texture2DVector>(releaseSetOnExit_.texture2Ds);

			//vertex shader
			SAFE_RELEASE_VECTOR<VertexShaderVector>(releaseSetOnExit_.vertexShaders);

			//pixel shader
			SAFE_RELEASE_VECTOR<PixelShaderVector>(releaseSetOnExit_.pixelShaders);

			//pixel shader
			SAFE_RELEASE_VECTOR<GeometryShaderVector>(releaseSetOnExit_.geometryShaders);

			//pixel shader
			SAFE_RELEASE_VECTOR<ComputeShaderVector>(releaseSetOnExit_.computeShaders);

			//buffer
			SAFE_RELEASE_VECTOR<BufferVector>(releaseSetOnExit_.buffers);

			//inputLeyout
			SAFE_RELEASE_VECTOR<InputLayoutVector>(releaseSetOnExit_.inputLayouts);

			//sampler state
			SAFE_RELEASE_VECTOR<SamplerStateVector>(releaseSetOnExit_.samplerStates);

			//blend state
			SAFE_RELEASE_VECTOR<BlendStateVector>(releaseSetOnExit_.blendStates);

			//raster state
			SAFE_RELEASE_VECTOR<RasterVector>(releaseSetOnExit_.rasterStates);

			//textures from file
			SAFE_RELEASE_MAP<ShaderResourceFilePool>(releaseSetOnExit_.texturePool);

			//model
			SAFE_DELETE_MAP<ModelPool>(releaseSetOnExit_.modelPool);

			Log_Info("resource mgr release all resource.");
		}
	};


};

#endif


