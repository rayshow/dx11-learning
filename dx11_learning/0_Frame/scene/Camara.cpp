#include<algorithm>
#include"camara.h"

#undef min
#undef max

using namespace ul;




inline void BaseCamara::UpdateProjectRelative(const XMMATRIX& view)
{
	XMVECTOR det;
	XMMATRIX project = XMLoadFloat4x4(&project_);
	XMMATRIX transposeProject = XMMatrixTranspose(project);
	XMMATRIX viewProject = XMMatrixMultiply(view, project);
	XMMATRIX transposeViewProject = XMMatrixTranspose(viewProject);
	XMMATRIX invViewProject = XMMatrixInverse(&det, viewProject);
	XMMATRIX transposeInvViewProject = XMMatrixTranspose(invViewProject);
	XMMATRIX rotateMatrix = XMLoadFloat3x3(&rotateMatrix_);
	XMMATRIX rotateProject = XMMatrixMultiply(rotateMatrix, project);
	XMMATRIX invRotateProject = XMMatrixInverse(&det, rotateProject);
	XMMATRIX transposeInvRotateProject = XMMatrixTranspose(invRotateProject);
	XMMATRIX transposeRotateProject = XMMatrixTranspose(rotateProject);
	XMStoreFloat4x4(&transposeRotateProject_, transposeRotateProject);
	XMStoreFloat4x4(&transposeProject_, transposeProject);
	XMStoreFloat4x4(&viewProject_, viewProject);
	XMStoreFloat4x4(&transposeViewProject_, transposeViewProject);
	XMStoreFloat4x4(&invViewProject_, invViewProject);
	XMStoreFloat4x4(&transposeInvViewProject_, transposeInvViewProject);
	XMStoreFloat4x4(&invRotateProject_, invRotateProject);
	XMStoreFloat4x4(&transposeInvRotateProject_, transposeRotateProject);
}

void BaseCamara::UpdateViewRelative(const XMMATRIX& view)
{
	XMVECTOR det;
	XMMATRIX invView = XMMatrixInverse(&det, view);
	XMMATRIX transposeView = XMMatrixTranspose(view);
	XMMATRIX transposeInvView = XMMatrixTranspose(invView);
	XMStoreFloat4x4(&view_, view);
	XMStoreFloat3x3(&rotateMatrix_, view);
	XMStoreFloat4x4(&invView_, invView);
	XMStoreFloat4x4(&transposeView_, transposeView);
	XMStoreFloat4x4(&transposeInvView_, transposeInvView);
}


void  BaseCamara::LookAt(XMFLOAT4& eye, XMFLOAT4& at)
{
	position_ = eye;
	lookDirection_ = at;
	XMVECTOR vpos = XMLoadFloat4(&eye);
	XMVECTOR vat = XMLoadFloat4(&at);
	XMVECTOR vup = XMVectorSet(0, 1, 0, 0);
	XMMATRIX view = XMMatrixLookAtLH(vpos, vat, vup);
	UpdateViewRelative(view);
	XMMATRIX invView = GetInvViewMatrix();
	XMVECTOR angleVector = invView.r[2];
	XMFLOAT4 angles;
	XMStoreFloat4(&angles, angleVector);
	yaw_   = atan2f(angles.x, angles.z);
	pitch_ = -atan2f(angles.y, angles.x*angles.x + angles.z*angles.z);
}


void  BaseCamara::SetProject(ECamaraProjectType type,
	float p, float q, float nearclip =0.1, float farclip =1000 )
{
	near_ = nearclip;
	far_ = farclip;
	projectType_ = type;
	projParameter_[0] = p;
	projParameter_[1] = q;
	XMMATRIX project;

	switch (type)
	{
	case eCamara_Perspective:
		project = XMMatrixPerspectiveFovLH(p, q, nearclip, farclip);
		XMStoreFloat4x4(&project_, project);
		break;
	case eCamara_Ortho:
		project = XMMatrixOrthographicLH(p, q, nearclip, farclip);
		XMStoreFloat4x4(&project_, project);
		break;
	default:
		Log_Err("project type not support!");
		assert(0);
		break;
	}
	UpdateProjectRelative(GetViewMatrix());
}


FirstPersonController::eCamaraKey FirstPersonController::keyMap(ulUint keycode)
{
	switch (keycode)
	{
	case 'A':
		return eCamaraMoveLeft;
	case 'D':
		return eCamaraMoveRight;
	case 'W':
		return eCamaraMoveForward;
	case 'S':
		return eCamaraMoveBackward;
	case 'Q':
		return eCamaraMoveUp;
	case 'E':
		return eCamaraMoveDown;
	case VK_SPACE:
		return eCamaraMoveReset;
	default:
		return eCamaraMoveMax;
		break;
	}
}

void FirstPersonController::ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
		eCamaraKey mappedKey;
		if (eCamaraMoveMax != (mappedKey = keyMap(wParam)))
		{
			if (uMsg == WM_KEYDOWN)
				keyDown_[mappedKey] = true;
			else
				keyDown_[mappedKey] = false;
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
		uMsg == WM_RBUTTONDOWN?(mouseDown_[eCamaraMouseRight] = true):1;
		uMsg == WM_MBUTTONDOWN?(mouseDown_[eCamaraMouseMiddle] = true):1;
		uMsg == WM_LBUTTONDOWN?(mouseDown_[eCamaraMouseLeft] = true):1;
		if (mouseDown_[eCamaraMouseLeft] || mouseDown_[eCamaraMouseMiddle] || mouseDown_[eCamaraMouseRight])
		{
			//Log_Info("capture");
			GetCursorPos(&lastMousePosition_);
			SetCapture(hWnd);
		}
		break;

	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_LBUTTONUP:
		mouseDown_[eCamaraMouseRight] = false;
		mouseDown_[eCamaraMouseMiddle] = false;
		mouseDown_[eCamaraMouseLeft] = false;
		if (!mouseDown_[eCamaraMouseLeft] && !mouseDown_[eCamaraMouseMiddle] && !mouseDown_[eCamaraMouseRight])
		{
			//Log_Info("release");
			ReleaseCapture();
		}
		break;

	case WM_CAPTURECHANGED:
		Log_Info("window capture the mouse.");
		break;
	}


}

void FirstPersonController::Update(float elapsedTime)
{
	//recover the position of camara
	if (keyDown_[eCamaraMoveReset])
		Reset();

	//move
	if (keyDown_[eCamaraMoveForward])
		moveDirection_.z += 1;
	if (keyDown_[eCamaraMoveBackward])
		moveDirection_.z -= 1;
	if (keyDown_[eCamaraMoveRight])
		moveDirection_.x += 1;
	if (keyDown_[eCamaraMoveLeft])
		moveDirection_.x -= 1;
	if (keyDown_[eCamaraMoveUp])
		moveDirection_.y += 1;
	if (keyDown_[eCamaraMoveDown])
		moveDirection_.y -= 1;
	moveDirection_.w = 0;

	float yawDelta = 0;
	float pitchDelta = 0;
	POINT ptCurMouseDelta = { 0, 0 };
	if (mouseDown_[eCamaraMouseLeft])
	{
		POINT ptCurMousePos;
		GetCursorPos(&ptCurMousePos);

		// Calc how far it's moved since last frame
		ptCurMouseDelta.x = ptCurMousePos.x - lastMousePosition_.x;
		ptCurMouseDelta.y = ptCurMousePos.y - lastMousePosition_.y;

		lastMousePosition_ = ptCurMousePos;
	}

	XMVECTOR moveDelta = XMLoadFloat4(&moveDirection_);
	moveDelta = XMVector3Normalize(moveDelta) * moveScaler_*elapsedTime;

	ptCurMouseDelta = ptCurMouseDelta ;
	yawDelta = ptCurMouseDelta.x * rotateScaler_*elapsedTime;
	pitchDelta = ptCurMouseDelta.y * rotateScaler_*elapsedTime;

	pCamara_->yaw_ += yawDelta;
	pCamara_->pitch_ += pitchDelta;

	//Log_Info("yaw:%f pitch:%f delta(%f %f)", yaw_, pitch_, yawDelta, pitchDelta);

	// Limit pitch to straight up or straight down
	pCamara_->pitch_ = std::max(-XM_PI / 2.0f, pCamara_->pitch_);
	pCamara_->pitch_ = std::min(XM_PI / 2.0f, pCamara_->pitch_);

	XMMATRIX camaraRot = XMMatrixRotationRollPitchYaw(pCamara_->pitch_, pCamara_->yaw_, 0);
	XMVECTOR localUp = XMVectorSet(0, 1, 0, 0);
	XMVECTOR localForward = XMVectorSet(0, 0, 1, 0);
	XMVECTOR worldUp = XMVector4Transform(localUp, camaraRot);
	XMVECTOR worldForward = XMVector4Transform(localForward, camaraRot);
	moveDelta = XMVector4Transform(moveDelta, camaraRot);

	XMVECTOR eyePos = XMLoadFloat4(&pCamara_->position_);
	eyePos += moveDelta;

	XMVECTOR lookAt = eyePos + worldForward;
	XMMATRIX view = XMMatrixLookAtLH(eyePos, lookAt, worldUp);
	
	pCamara_->UpdateAllMatrix(view);
	XMStoreFloat4(&pCamara_->position_, eyePos);
	
	
	moveDirection_ = XMFLOAT4(0, 0, 0, 0);
}

