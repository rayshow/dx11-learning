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

	//回收集
	class ResourceMgr : public Singleton<ResourceMgr>
	{
	public:
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

		typedef map< string, ID3D11ShaderResourceView*>      ShaderResourceFilePool;
		typedef map< string, BaseModel*>					 ModelPool;

		//view iter
		typedef ShaderResViewVector::iterator      ShaderResViewVectorIter;
		typedef RenderTargetViewVector::iterator   RenderTargetViewVectorIter;
		typedef DepthStencilViewVector::iterator   DepthStencilViewVectorIter;
		typedef UnorderedAccessViewVector::iterator UnorderedAccessViewIter;
		typedef Texture2DVector::iterator            Texture2DVectorIter;

		//shader iter
		typedef VertexShaderVector::iterator       VertexShaderVectorIter;
		typedef PixelShaderVector::iterator        PixelShaderVectorIter;
		typedef GeometryShaderVector::iterator     GeometryShaderVectorIter;
		typedef ComputeShaderVector::iterator      ComputeShaderVectorIter;

		//state iter
		typedef SamplerStateVector::iterator       SamplerStateVectorIter;
		typedef BlendStateVector::iterator         BlendStateVectorIter;
		typedef InputLayoutVector::iterator        InputLayoutVectorIter;
		typedef BufferVector::iterator             BufferVectorIter;
		typedef RasterVector::iterator             RasterVectorIter;
		typedef ShaderResourceFilePool::iterator   TexturePoolIter;
		typedef ModelPool::iterator                ModelPoolIter;

		//窗口重置后需要回收
		struct sRecyclePerReleasingSwapChain
		{
			ShaderResViewVector    shaderResViews;
			RenderTargetViewVector renderTargetViews;
			DepthStencilViewVector depthStencilViews;
			Texture2DVector        texture2Ds;
		};


		//退出回收
		struct sReleaseOnExit
		{
			ShaderResViewVector    shaderResViews;
			RenderTargetViewVector renderTargetViews;
			DepthStencilViewVector depthStencilViews;
			UnorderedAccessViewVector unorderedViews;
			Texture2DVector        texture2Ds;

			VertexShaderVector     vertexShaders;
			PixelShaderVector      pixelShaders;
			GeometryShaderVector   geometryShaders;
			ComputeShaderVector    computeShaders;

			SamplerStateVector     samplerStates;
			BlendStateVector       blendStates;
			InputLayoutVector      inputLayouts;
			BufferVector           buffers;
			RasterVector           rasterStates;

			//pool
			ShaderResourceFilePool texturePool;
			ModelPool              modelPool;
		};

		//类型
		enum eRecycleItem
		{
			eRecyclePerSwapChain,
			eReleaseOnExit
		};
	private:
		sRecyclePerReleasingSwapChain  m_sResRecyclePerSwapChain;
		sReleaseOnExit                 m_sResReleaseOnExit;
		ID3D11Device*                  m_device;
		ID3D11DeviceContext*           m_context;
		string                         resourceBasePath_;
	public:
		ResourceMgr(){}
	private:
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
			HRESULT hr =D3DX11CompileFromFileA(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
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


		//创建顶点着色器
		inline ID3D11VertexShader*
			createVertexShader(ID3DBlob * blob)
		{
				ID3D11VertexShader *vertexShader;
				Fail_Return_Null(m_device->CreateVertexShader(
					blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vertexShader));
				m_sResReleaseOnExit.vertexShaders.push_back(vertexShader);
				return vertexShader;
			}

		//创建inputLayout
		inline ID3D11InputLayout*
			createInputLayout(const D3D11_INPUT_ELEMENT_DESC *descs, UINT count, ID3DBlob* pBlob)
		{
				ID3D11InputLayout *layout;
				Fail_Return_Null_With_Msg(
					m_device->CreateInputLayout(descs, count, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &layout),
					"create input layout error."
				);
				m_sResReleaseOnExit.inputLayouts.push_back(layout);
				return layout;
			}

		//创建片元着色器
		inline ID3D11PixelShader*
			createPixelShader(ID3DBlob * blob)
		{
				ID3D11PixelShader *pixelShader;
				Fail_Return_Null(m_device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader));
				m_sResReleaseOnExit.pixelShaders.push_back(pixelShader);
				return pixelShader;
			}


		//创建计算着色器
		inline ID3D11ComputeShader*
			createComputeShader(ID3DBlob * blob)
		{
				ID3D11ComputeShader *computerShader;
				Fail_Return_Null(m_device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &computerShader));
				m_sResReleaseOnExit.computeShaders.push_back(computerShader);
				return computerShader;
		}

		//创建几何着色器
		inline ID3D11GeometryShader*
			createGeometryShader(ID3DBlob * blob)
		{
			ID3D11GeometryShader *geoShader;
			Fail_Return_Null(m_device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &geoShader));
			m_sResReleaseOnExit.geometryShaders.push_back(geoShader);
			return geoShader;
		}


	public:
		
		//初始化
		inline void
			init(ID3D11Device* dev, ID3D11DeviceContext* context){
			m_device = dev;
			m_context = context;
			Log_Info("resource manager initialized.");
		}

		inline void
			init(ID3D11Device* dev, string resourceBasePath){
				m_device = dev;
				resourceBasePath_ = resourceBasePath;
				Log_Info("resource manager initialized.");
			}

		inline void 
			SetResourceBasePath(const string& resourcePath)
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

		inline string GetResourceBasePath() const 
		{
			return resourceBasePath_;
		}

		//mapping constant buffer
		inline bool MappingBufferWriteOnly(ID3D11Buffer* constBuffer, void* buffer, ulUint bufferSize)
		{
			D3D11_MAPPED_SUBRESOURCE MappedResource;
			memset(&MappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
			Fail_Return_False(m_context->Map(constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
			memcpy(MappedResource.pData, buffer, bufferSize);
			m_context->Unmap(constBuffer, 0);

			return true;
		}

		
		//rasterize state
		inline ID3D11RasterizerState*
			CreateRasterState(D3D11_RASTERIZER_DESC& desc)
		{
				ID3D11RasterizerState *rasterState;
				Fail_Return_Null_With_Msg(
					m_device->CreateRasterizerState(&desc, &rasterState),
					"create raster state error, parameter maybe incorrect."
					);
				m_sResReleaseOnExit.rasterStates.push_back(rasterState);
				return rasterState;
			}

		//创建shader资源视图
		inline ID3D11ShaderResourceView*
			CreateShaderResourceView(
			ID3D11Resource* from,
			D3D11_SHADER_RESOURCE_VIEW_DESC* desc,
			eRecycleItem em = eRecycleItem::eReleaseOnExit)
		{
				ID3D11ShaderResourceView *resView;
				Fail_Return_Null_With_Msg(
					m_device->CreateShaderResourceView(from, desc, &resView),
					"create Shader resource view error, parameter maybe incorrect."
					);

				switch (em)
				{
				case eRecyclePerSwapChain:
					m_sResRecyclePerSwapChain.shaderResViews.push_back(resView);
					break;
				case eReleaseOnExit:
					m_sResReleaseOnExit.shaderResViews.push_back(resView);
					break;
				default:
					assert(0);
				}
				return resView;
			}


		//创建渲染视图
		inline ID3D11RenderTargetView*
			CreateRenderTargetView(
			ID3D11Resource* from,
			D3D11_RENDER_TARGET_VIEW_DESC* desc,
			eRecycleItem em = eRecycleItem::eReleaseOnExit)
		{
				ID3D11RenderTargetView *rtView;

				Fail_Return_Null_With_Msg(
					m_device->CreateRenderTargetView(from, desc, &rtView),
					"reate render target view  error, parameter maybe incorrect."
					);

				switch (em)
				{
				case eRecyclePerSwapChain:
					m_sResRecyclePerSwapChain.renderTargetViews.push_back(rtView);
					break;
				case eReleaseOnExit:
					m_sResReleaseOnExit.renderTargetViews.push_back(rtView);
					break;
				default:
					assert(0);
				}

				return rtView;
			}

		//创建纹理
		inline ID3D11ShaderResourceView*
			CreateTextureFromFile(const string& fileName)
		{
				ID3D11ShaderResourceView *rt;
				TexturePoolIter find = m_sResReleaseOnExit.texturePool.find(fileName);

				//exists
				if (find != m_sResReleaseOnExit.texturePool.end())
				{
					Log_Info("read texture %s is existed in texture pool", fileName.c_str());
					return find->second;
				}

				Fail_Return_Null_With_Msg(
					D3DX11CreateShaderResourceViewFromFileA(m_device, fileName.c_str(), nullptr, nullptr, &rt, nullptr),
					"can't find file %s or format not support.", fileName.c_str());

				m_sResReleaseOnExit.texturePool.insert(std::make_pair(fileName, rt));
				Log_Info("read texture %s ok", fileName.c_str());
				return rt;
			}

		inline BaseModel* CreateModelFromFile(const string& fileName)
		{
			ModelPoolIter find = m_sResReleaseOnExit.modelPool.find(fileName);
			//exists
			if (find != m_sResReleaseOnExit.modelPool.end())
			{
				Log_Info("model %s is existed in texture pool", fileName.c_str());
				return find->second;
			}

			CommonModelLoader loader;
			SModelData data;

			Log_Info("loading model %s ", fileName.c_str());
			False_Return_Null_With_Msg(
				loader.LoadFile(resourceBasePath_,  fileName, data),
				"loading model %s error.", fileName.c_str()
			);

			BaseModel* model = new BaseModel();
			Null_Return_Null(model);
			model->Create(m_device, data);
			ModelData_Free(data);

			m_sResReleaseOnExit.modelPool.insert(std::make_pair(fileName, model));
			Log_Info("read model %s ok", fileName.c_str());
			return model;
		}

		//深度缓冲视图
		inline ID3D11DepthStencilView*
			CreateDepthStencilView(
			ID3D11Resource* from,
			D3D11_DEPTH_STENCIL_VIEW_DESC* desc,
			eRecycleItem em = eRecycleItem::eReleaseOnExit)
		{
				ID3D11DepthStencilView *dsView;

				Fail_Return_Null_With_Msg(
					m_device->CreateDepthStencilView(from, desc, &dsView),
					"create depth stencil view error, parameter maybe incorrect."
					);

				switch (em)
				{
				case eRecyclePerSwapChain:
					m_sResRecyclePerSwapChain.depthStencilViews.push_back(dsView);
					break;
				case eReleaseOnExit:
					m_sResReleaseOnExit.depthStencilViews.push_back(dsView);
					break;
				default:
					assert(0);
				}

				return dsView;
			}

		//无序视图
		inline ID3D11UnorderedAccessView*
			CreateUnorderedAccessView(
			ID3D11Resource* from,
			const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc,
			eRecycleItem em)
		{
				ID3D11UnorderedAccessView *uav;
				Fail_Return_Null_With_Msg(
					m_device->CreateUnorderedAccessView(from, &desc, &uav),
					"create unordered access view error, parameter maybe incorrect."
					);

				m_sResReleaseOnExit.unorderedViews.push_back(uav);
				return uav;
		}

		inline ID3D11Texture2D*
			CreateTexture2DNoData(
			const D3D11_TEXTURE2D_DESC& desc,
			eRecycleItem em = eRecycleItem::eReleaseOnExit)
		{
				ID3D11Texture2D *tex2d;
				m_device->CreateTexture2D(&desc, nullptr, &tex2d);
				switch (em)
				{
				case eRecyclePerSwapChain:
					m_sResRecyclePerSwapChain.texture2Ds.push_back(tex2d);
					break;
				case eReleaseOnExit:
					m_sResReleaseOnExit.texture2Ds.push_back(tex2d);
					break;
				default:
					assert(0);
				}
				return tex2d;
		}


		//编译和创建顶点着色器
		inline ID3D11VertexShader*
			CreateVertexShader(
			const char* fileName,
			const char* functionName,
			const char*  v)
		{
				ID3D11VertexShader *vs = nullptr;
				AutoReleasePtr<ID3DBlob> blobPtr;
				False_Return_Null_With_Msg(
					CompileShaderFromFile(fileName, functionName, v, blobPtr.GetPtr()),
					"compile vertex shader %s, %s error.", fileName, functionName
					);
				Null_Return_Null_With_Msg(
					(vs = this->createVertexShader(blobPtr.Get())),
					"compile  vertex shader %s function %s error", fileName, functionName
					);
				return vs;
			}

		//编译和创建顶点着色器
		inline ID3D11VertexShader*
			CreateVertexShaderFromResourceBasePath(
			const char* reletiveName,
			const char* functionName,
			const char*  v)
		{
			ID3D11VertexShader *vs = nullptr;
			AutoReleasePtr<ID3DBlob> blobPtr;
			string fullPathName = resourceBasePath_ + reletiveName;

			False_Return_Null_With_Msg(
				CompileShaderFromFile(fullPathName.c_str(), functionName, v, blobPtr.GetPtr()),
				"compile vertex shader %s, %s error.", fullPathName.c_str(), functionName
				);
			Null_Return_Null_With_Msg(
				(vs = this->createVertexShader(blobPtr.Get())),
				"compile  vertex shader %s function %s error", fullPathName.c_str(), functionName
				);
			return vs;
		}


		//编译和创建有顶点格式的顶点着色器
		inline ID3D11VertexShader*
			CreateVertexShaderAndInputLayout(
			const char* fileName,
			const char* functionName,
			const char* v,
			const D3D11_INPUT_ELEMENT_DESC* loDesc,
			int descSize,
			ID3D11InputLayout** layout
			)
		{
				ID3D11VertexShader *vs = nullptr;
				AutoReleasePtr<ID3DBlob> blobPtr;
				False_Return_Null_With_Msg(
					CompileShaderFromFile(fileName, functionName, v, blobPtr.GetPtr()),
					"compile shader file %s ,function %s error", fileName, functionName
					);
				Null_Return_Null_With_Msg(
					(vs = this->createVertexShader(blobPtr.Get())),
					"create shader file %s ,function %s error", fileName, functionName
					);
				Null_Return_Null_With_Msg(
					(*layout = this->createInputLayout(loDesc, descSize, blobPtr.Get())),
					"create shader file %s ,function %s vertex layout error", fileName, functionName
					);
				return vs;
		}

		//编译和创建有顶点格式的顶点着色器
		inline ID3D11VertexShader*
			CreateVertexShaderAndInputlayoutFromResourceBasePath(
			const char* reletivePath,
			const char* functionName,
			const char* v,
			const D3D11_INPUT_ELEMENT_DESC* loDesc,
			int descSize,
			ID3D11InputLayout** layout
			)
		{
			ID3D11VertexShader *vs = nullptr;
			AutoReleasePtr<ID3DBlob> blobPtr;
			string fullPathName = resourceBasePath_ + reletivePath;

			False_Return_Null_With_Msg(
				CompileShaderFromFile(fullPathName.c_str(), functionName, v, blobPtr.GetPtr()),
				"compile shader file %s ,function %s error", fullPathName.c_str(), functionName
				);
			Null_Return_Null_With_Msg(
				(vs = this->createVertexShader(blobPtr.Get())),
				"create shader file %s ,function %s error", fullPathName.c_str(), functionName
				);
			Null_Return_Null_With_Msg(
				(*layout = this->createInputLayout(loDesc, descSize, blobPtr.Get())),
				"create shader file %s ,function %s vertex layout error", fullPathName.c_str(), functionName
				);
			return vs;
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
			CreatePixelShaderFromResourceBasePath(
			const char* reletivePath,
			const char* functionName,
			const char* v)
		{
			ID3D11PixelShader *ps = nullptr;
			AutoReleasePtr<ID3DBlob> blobPtr;
			string fullPathName = resourceBasePath_ + reletivePath;

			False_Return_Null_With_Msg(
				CompileShaderFromFile(fullPathName.c_str(), functionName, v, blobPtr.GetPtr()),
				"compile  shader file %s ,pixel function %s error", fullPathName.c_str(), functionName
				);
			Null_Return_Null_With_Msg(
				((ps = this->createPixelShader(blobPtr.Get()))),
				"create  shader file %s ,pixel function %s error", fullPathName.c_str(), functionName
				);
			return ps;

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
				Fail_Return_Null_With_Msg(
					m_device->CreateSamplerState(&desc, &samplerState),
					"create sample state error"
					);
				m_sResReleaseOnExit.samplerStates.push_back(samplerState);
				return samplerState;
		}

		//三线性采样
		inline ID3D11SamplerState*
			CreateTrilinearSamplerState()
		{
			D3D11_SAMPLER_DESC SamDesc;
			SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			SamDesc.MipLODBias = 0.0f;
			SamDesc.MaxAnisotropy = 1;
			SamDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
			SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0.0;
			SamDesc.MinLOD = 0;
			SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
			SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			return this->CreateSamplerState(SamDesc);
		}

		inline ID3D11SamplerState* 
			CreateLinearSamplerState()
		{
			D3D11_SAMPLER_DESC SamDesc;
			SamDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			SamDesc.MipLODBias = 0.0f;
			SamDesc.MaxAnisotropy = 1;
			SamDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
			SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0.0;
			SamDesc.MinLOD = 0;
			SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
			SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			return this->CreateSamplerState(SamDesc);
		}

		//创建混合状态
		inline ID3D11BlendState*
			CreateBlendState(D3D11_BLEND_DESC &desc)
		{
				ID3D11BlendState *blendState;
				Fail_Return_Null_With_Msg(
					m_device->CreateBlendState(&desc, &blendState),
					"create blend state error"
					);
				m_sResReleaseOnExit.blendStates.push_back(blendState);
				return blendState;
			}

		//创建缓存
		inline ID3D11Buffer*
			CreateBuffer(D3D11_BUFFER_DESC& desc, const D3D11_SUBRESOURCE_DATA *data)
		{
				ID3D11Buffer *buffer;
				Fail_Return_Null_With_Msg(
					m_device->CreateBuffer(&desc, data, &buffer),
					"create buffer  error"
					);
				m_sResReleaseOnExit.buffers.push_back(buffer);
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

				Null_Return_Null_With_Msg(
					(vb = this->CreateBuffer(vbd, &vdata)),
					"create vertex buffer error."
					);
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
			SAFE_RELEASE_VECTOR<ShaderResViewVector>(m_sResRecyclePerSwapChain.shaderResViews);

			//release RT
			SAFE_RELEASE_VECTOR<RenderTargetViewVector>(m_sResRecyclePerSwapChain.renderTargetViews);

			//release DSV
			SAFE_RELEASE_VECTOR<DepthStencilViewVector>(m_sResRecyclePerSwapChain.depthStencilViews);

			//release texuture2d
			SAFE_RELEASE_VECTOR<Texture2DVector>(m_sResRecyclePerSwapChain.texture2Ds);
		}

		//app退出时释放
		inline void ReleaseLoadedResourceOnExit()
		{
			//release SRV
			SAFE_RELEASE_VECTOR<ShaderResViewVector>(m_sResReleaseOnExit.shaderResViews);

			//release RT
			SAFE_RELEASE_VECTOR<RenderTargetViewVector>(m_sResReleaseOnExit.renderTargetViews);

			//release DSV
			SAFE_RELEASE_VECTOR<DepthStencilViewVector>(m_sResReleaseOnExit.depthStencilViews);

			//release UAV
			SAFE_RELEASE_VECTOR<UnorderedAccessViewVector>(m_sResReleaseOnExit.unorderedViews);

			//release texuture2d
			SAFE_RELEASE_VECTOR<Texture2DVector>(m_sResReleaseOnExit.texture2Ds);

			//vertex shader
			SAFE_RELEASE_VECTOR<VertexShaderVector>(m_sResReleaseOnExit.vertexShaders);

			//pixel shader
			SAFE_RELEASE_VECTOR<PixelShaderVector>(m_sResReleaseOnExit.pixelShaders);

			//pixel shader
			SAFE_RELEASE_VECTOR<GeometryShaderVector>(m_sResReleaseOnExit.geometryShaders);

			//pixel shader
			SAFE_RELEASE_VECTOR<ComputeShaderVector>(m_sResReleaseOnExit.computeShaders);

			//buffer
			SAFE_RELEASE_VECTOR<BufferVector>(m_sResReleaseOnExit.buffers);

			//inputLeyout
			SAFE_RELEASE_VECTOR<InputLayoutVector>(m_sResReleaseOnExit.inputLayouts);

			//sampler state
			SAFE_RELEASE_VECTOR<SamplerStateVector>(m_sResReleaseOnExit.samplerStates);

			//blend state
			SAFE_RELEASE_VECTOR<BlendStateVector>(m_sResReleaseOnExit.blendStates);

			//raster state
			SAFE_RELEASE_VECTOR<RasterVector>(m_sResReleaseOnExit.rasterStates);

			//textures from file
			SAFE_RELEASE_MAP<ShaderResourceFilePool>(m_sResReleaseOnExit.texturePool);

			//model
			SAFE_DELETE_MAP<ModelPool>(m_sResReleaseOnExit.modelPool);

			Log_Info("resource mgr release all resource.");
		}
	};


};

#endif


