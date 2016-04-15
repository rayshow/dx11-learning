#ifndef UL_POSTPROCESS_HEADER__
#define UL_POSTPROCESS_HEADER__

#include<string>
#include<vector>
#include<d3d11.h>
#include<D3DX11tex.h>

#include"MeshRender.h"

namespace ul
{
	using namespace std;

	class PostProcess
	{
		struct SPostProcess_Parameter
		{
			float  mipLevel;
			float  padding[3];
		};
	private:
		ID3D11RenderTargetView*		renderTarget_;
		ID3D11ShaderResourceView*   needProcessSRV_;
		ID3D11ShaderResourceView*   nextStepProcessSRV_;
		ID3D11VertexShader*         fullScreenVs_;
		ID3D11PixelShader*          fullScreenPs_;
		ID3D11Texture2D*            srvOwnerTexture_;
		ID3D11Buffer*               parameterBuffer_;
		ulUint                      width_;
		ulUint                      height_;
		DXGI_FORMAT                 format_;

	public:
		PostProcess() : renderTarget_(nullptr), needProcessSRV_(nullptr), nextStepProcessSRV_(nullptr),fullScreenVs_(nullptr),
			fullScreenPs_(nullptr), srvOwnerTexture_(nullptr), parameterBuffer_(nullptr){}
		~PostProcess(){}
	public:
		void SetShaderResourceView(ID3D11ShaderResourceView* shaderResourceView)
		{
			this->needProcessSRV_ = shaderResourceView;
		}

		ID3D11ShaderResourceView* GetNextStepShaderResource()
		{
			return nextStepProcessSRV_;
		}

		bool PostProcess::Create(
			const string& fileName,
			const string& psFunction,
			DXGI_FORMAT format,
			ulUint width, ulUint height,
			ID3D11Texture2D *owner ,
			ulUint mipLevel );

		void Process(ID3D11DeviceContext* context) 
		{
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			context->VSSetShader(fullScreenVs_, nullptr, 0);
			context->PSSetShader(fullScreenPs_, nullptr, 0);
			context->PSSetConstantBuffers(0, 1, &parameterBuffer_);
			context->OMSetRenderTargets(0, &renderTarget_, nullptr);
			context->PSSetShaderResources(0, 1, &needProcessSRV_);
			context->Draw(3, 0);
		}
	};


	class PostPresent
	{
	private:
		ID3D11ShaderResourceView*  needPresent_;
		ID3D11VertexShader*        fullScreenVs_;
		ID3D11PixelShader*         fullScreenPs_;
		ulUint                     width_;
		ulUint                     height_;
	public:
		PostPresent() :needPresent_(nullptr), 
			fullScreenVs_(nullptr),
			fullScreenPs_(nullptr),
			width_(0), height_(0){}
		~PostPresent(){}
	public:
		bool Create();

		void SetNeedPresentTexture(ID3D11ShaderResourceView* view)
		{
			this->needPresent_ = view;
		}
		void Present(ID3D11DeviceContext* context, ID3D11RenderTargetView* mainRT)
		{
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			context->VSSetShader(fullScreenVs_, nullptr, 0);
			context->PSSetShader(fullScreenPs_, nullptr, 0);
			context->OMSetRenderTargets(1, &mainRT, nullptr);
			context->PSSetShaderResources(0, 1, &needPresent_);
			context->Draw(3, 0);
		}
	};


	class HdrPresentProcess : public PostProcess
	{
	public:
		~HdrPresentProcess(){}
	public:
		bool Create(ulUint width, ulUint height);
	};

	class PostProcessChain
	{
	public:
		enum EPostProcessType{
			ePostProcess_PresentHDR = 0,
			ePostProcess_FXAA,
		};
	private:
		std::vector<PostProcess*> processes_;
		PostPresent               present_;
		ID3D11RenderTargetView*   srcRenderTarget_;
		ID3D11ShaderResourceView* srcResourceView_;
		ID3D11Texture2D*          srcTexture_;
		ulUint                    width_;
		ulUint                    height_;
	public:
		PostProcessChain()
		{

		}
		~PostProcessChain()
		{
			this->RemoveAllProcess();
		}
		void RemoveAllProcess()
		{
			for (ulUint i = 0; i < processes_.size(); ++i)
			{
				Safe_Delete(processes_.at(i));
			}
			processes_.erase(processes_.begin(), processes_.end());
		}
		bool Create(ulUint width, ulUint height);

	public:
		void ClearBackground(ID3D11DeviceContext* context)
		{
			const float color[4] = {0,0,0,0};
			context->ClearRenderTargetView(srcRenderTarget_, color);
		}
		void BindAsRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthView)
		{
			context->OMSetRenderTargets(1, &srcRenderTarget_, depthView);
		}

		void AddPostProcess(PostProcess* process)
		{
			assert(process != nullptr);
			if (processes_.size() == 0)
			{
				process->SetShaderResourceView(srcResourceView_);
			}
			else{
				process->SetShaderResourceView(processes_.at(processes_.size() - 1)->GetNextStepShaderResource());
				
			}
			processes_.push_back(process);
			present_.SetNeedPresentTexture(process->GetNextStepShaderResource());
		}

		void Process(ID3D11DeviceContext* context)
		{
			for (ulUint i = 0; i < processes_.size(); ++i)
			{
				processes_.at(i)->Process(context);
			}
		}

		void Present(ID3D11DeviceContext* context, ID3D11RenderTargetView* mainRT)
		{
			present_.Present(context, mainRT);
		}

		void AddPostProcess(EPostProcessType type)
		{
			HdrPresentProcess *process = nullptr;

			switch (type)
			{
			//hdr present
			case ePostProcess_PresentHDR:
				process = Ul_New HdrPresentProcess();
				process->Create(width_, height_);
				if (Null(process))
				{
					Log_Err("out of memory.");
				}
				this->AddPostProcess(process);
				break;

			case ul::PostProcessChain::ePostProcess_FXAA:
				break;
			default:
				break;
			}
		}
	};


};


#endif