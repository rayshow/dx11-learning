#ifndef CAMARA_HEADER__
#define CAMARA_HEADER__

#include<Windows.h>
#include<xnamath.h>

#include"../util/UlHelper.h"

namespace ul
{
	class BaseCamara
	{
		friend class FirstPersonController;

	public:
		BaseCamara(){}
		~BaseCamara(){}

		enum ECamaraProjectType{
			eCamara_Perspective,
			eCamara_Ortho,
		};
	protected:
		XMFLOAT4X4			view_;
		XMFLOAT4X4			project_;
		XMFLOAT4X4          viewProject_;
		XMFLOAT4X4          invView_;
		XMFLOAT4X4          rotateProject_;
		XMFLOAT4X4          invRotateProject_;
		XMFLOAT4X4          invProject_;
		XMFLOAT4X4          invViewProject_;
		XMFLOAT3X3          rotateMatrix_;

		XMFLOAT4X4          transposeView_;
		XMFLOAT4X4          transposeInvView_;
		XMFLOAT4X4          transposeProject_;
		XMFLOAT4X4          transposeViewProject_;
		XMFLOAT4X4          transposeInvViewProject_;
		XMFLOAT4X4          transposeRotateProject_;
		XMFLOAT4X4          transposeInvRotateProject_;

		XMFLOAT4			position_;
		XMFLOAT4			rotate_;
		XMFLOAT4			lookDirection_;

		//angle
		ulFloat			    yaw_;
		ulFloat			    pitch_;

		//
		ulFloat             near_;
		ulFloat             far_;

		ECamaraProjectType  projectType_;
		ulFloat             projParameter_[2];

	public:
		inline ulFloat GetYaw() { return yaw_; }

		inline void SetYaw(ulFloat yaw) { this->yaw_ = yaw; }

		inline ulFloat GetPitch(){ return pitch_; }

		inline void SetPitch(ulFloat pitch) { this->pitch_ = pitch; }

		XMFLOAT4& GetEyePosStoreType() { return position_; }
		XMVECTOR GetEyePos() { return XMLoadFloat4(&position_); }
		float* GetEyePosStorePtr(){ return (float*)(&position_);  }

		inline XMMATRIX GetViewMatrix(){ return XMLoadFloat4x4(&view_); }

		inline XMMATRIX GetProjectMatrix(){ return XMLoadFloat4x4(&project_); }

		inline XMMATRIX GetViewProjectMatrix(){ return XMLoadFloat4x4(&viewProject_); }

		inline XMMATRIX GetInvViewMatrix() { return XMLoadFloat4x4(&invView_); }

		inline XMMATRIX GetInvViewProjectMatrix() { return XMLoadFloat4x4(&invViewProject_); }

		inline XMFLOAT4X4& GetInvRotateProjectStoreType() { return transposeInvRotateProject_; }
		inline float* GetInvRotateProjectStorePtr(){ return (float*)&invRotateProject_; }

		inline XMFLOAT4X4& GetInvViewProjectStoreType() { return transposeInvViewProject_; }
		inline float* GetInvViewProjectStorePtr(){ return (float*)&invViewProject_;  }

		inline XMFLOAT4X4&  GetViewStoreType() { return transposeView_; }
		inline float* GetViewStorePtr(){ return (float*)&view_; }

		inline XMFLOAT4X4&  GetProjectStoreType(){ return transposeProject_; }
		inline float* GetProjectStorePtr(){return (float*)&project_; }

		inline XMFLOAT4X4& GetViewProjectStoreType() {  return transposeViewProject_;  }
		inline float* GetViewProjectStorePtr()  { return (float*)&viewProject_; }
		
		inline XMFLOAT4X4& GetRotateProjectStoreType()  { return transposeRotateProject_; }
		inline float* GetRotateProjectStorePtr(){ return (float*)&rotateProject_;  }

		//update view invView transposeView 
	    void UpdateViewRelative(const XMMATRIX& view);

		void UpdateAllMatrix(const XMMATRIX& view) { UpdateViewRelative(view); UpdateProjectRelative(view); }

		void UpdateProjectRelative(const XMMATRIX& view);

		void   SetProject(ECamaraProjectType projectType, float , float, float, float);

		void   LookAt(XMFLOAT4& eye, XMFLOAT4& at);
	};  //BaseCamara


	class FirstPersonController
	{
	public:
		enum eCamaraKey{
			eCamaraMoveForward = 0,
			eCamaraMoveBackward,
			eCamaraMoveUp,
			eCamaraMoveDown,
			eCamaraMoveLeft,
			eCamaraMoveRight,
			eCamaraMoveReset,
			eCamaraMoveMax,
		};
		enum eMouseClick{
			eCamaraMouseLeft = 0,
			eCamaraMouseMiddle,
			eCamaraMouseRight,
			eCamaraMouseMax
		};
	private:
		ulFloat     rotateScaler_;
		ulFloat     moveScaler_;
		bool        keyDown_[eCamaraMoveMax];
		bool        mouseDown_[eCamaraMouseMax];
		XMFLOAT4    moveDirection_;
		POINT       lastMousePosition_;
		BaseCamara *pCamara_;

	public:
		FirstPersonController() :
			rotateScaler_(1),
			moveScaler_(50),
			pCamara_(nullptr)
		{
			
		}

		void SetRotateAndMoveScaler(float rotateScaler = 1.0f, float moveScaler = 50.0f)
		{
			this->rotateScaler_ = rotateScaler;
			this->moveScaler_ = moveScaler;
		}

		void SetCamara(BaseCamara* camara)
		{
			this->pCamara_ = camara;
		}
	protected:
		virtual eCamaraKey keyMap(ulUint keycode);
		void Reset(){}
	public:
		void Update(ulFloat elapsedTime);
		void ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	}; // FirstPersonCamara


}; //ul






#endif