#ifndef CAMARA_HEADER__
#define CAMARA_HEADER__

#include<Windows.h>
#include<xnamath.h>

#include"util/tools.h"

namespace ul
{
	class BaseCamara
	{
		friend class FirstPersonController;

	public:
		BaseCamara():
			mainCamara(false){}
		virtual ~BaseCamara(){}

		enum ECamaraProjectType{
			eCamara_Perspective,
			eCamara_Ortho,
		};
	protected:
		XMFLOAT4X4			view_;
		XMFLOAT4X4			project_;
		XMFLOAT4			position_;
		XMFLOAT4			rotate_;
		XMFLOAT4			lookDirection_;
		ulFloat			    yaw_;
		ulFloat			    pitch_;
		ECamaraProjectType  projectType_;
		ulFloat             near_;
		ulFloat             far_;
		ulFloat             projParam_[2];
		ulBit               mainCamara;

	public:
		inline XMFLOAT4X4  GetTransposeViewMatrix()
		{
			return TransposeMatrix(&view_);
		}
		inline XMFLOAT4X4  GetTransposeProjectMatrix()
		{
			return TransposeMatrix(&project_);
		}

		inline XMFLOAT4X4& GetViewMatrix()
		{
			return view_;
		}
		inline XMFLOAT4X4& GetProjectMatrix()
		{
			return project_;
		}
		inline XMFLOAT4X4  TransposeMatrix(const XMFLOAT4X4* matrix)
		{
			XMMATRIX transpose = XMLoadFloat4x4(matrix);
			XMFLOAT4X4 result;
			transpose = XMMatrixTranspose(transpose);
			XMStoreFloat4x4(&result, transpose);
			return result;
		}

		void   SetProject(ECamaraProjectType, float, float, float, float);
		XMFLOAT4& GetEyePos()
		{
			return position_;
		}

		void   LookTo(XMFLOAT4& eye, XMFLOAT4& dir);
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