#ifndef CAMARA_HEADER__
#define CAMARA_HEADER__

#include<Windows.h>
#include<xnamath.h>

#include"util/tools.h"
#include"Application.h"
#include"Input.h"

namespace ul
{
	class BaseCamara
	{
	public:
		BaseCamara():
			mainCamara(false){}
		virtual ~BaseCamara(){}
		enum eCamaraProjectType{
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
		eCamaraProjectType  projectType_;
		float               near_;
		float               far_;
		float               projParam_[2];
		bool                mainCamara;

	public:
		inline XMFLOAT4X4  GetTransposeViewMatrix();
		inline XMFLOAT4X4  GetTransposeProjectMatrix();
		inline XMFLOAT4X4& GetViewMatrix();
		inline XMFLOAT4X4& GetProjectMatrix();
		inline XMFLOAT4X4  TransposeMatrix(const XMFLOAT4X4* matrix);
		void   SetProject(eCamaraProjectType, float, float, float, float);
		XMFLOAT4& GetEyePos();
		void   LookTo(XMFLOAT4& eye, XMFLOAT4& dir);
		void   LookAt(XMFLOAT4& eye, XMFLOAT4& at);
	};

	inline XMFLOAT4X4& BaseCamara::GetViewMatrix()
	{
		return view_;
	}
	inline XMFLOAT4X4& BaseCamara::GetProjectMatrix()
	{
		return project_;
	}

	XMFLOAT4X4 BaseCamara::GetTransposeViewMatrix()
	{
		return TransposeMatrix(&view_);
	}

	XMFLOAT4X4 BaseCamara::GetTransposeProjectMatrix()
	{
		return TransposeMatrix(&project_);
	}

	inline XMFLOAT4X4 BaseCamara::TransposeMatrix(const XMFLOAT4X4* matrix)
	{
		XMMATRIX transpose = XMLoadFloat4x4(matrix);
		XMFLOAT4X4 result;
		transpose = XMMatrixTranspose(transpose);
		XMStoreFloat4x4(&result, transpose);
		return result;
	}

	inline XMFLOAT4& BaseCamara::GetEyePos()
	{
		return position_;
	}

	class FirstPersonCamara: public BaseCamara
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
		ulFloat    rotateScaler_;
		ulFloat    moveScaler_;
		bool       keyDown_[eCamaraMoveMax];
		bool       mouseDown_[eCamaraMouseMax];
		XMFLOAT4   moveDirection_;
		POINT      lastMousePosition_;
	public:
		FirstPersonCamara():
			rotateScaler_(1),
			moveScaler_(10)
		{}
	protected:
		virtual eCamaraKey keyMap(ulUint keycode);
		void Reset(){}
	public:
		void Update(ulFloat elapsedTime);
		void ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};
};



#endif