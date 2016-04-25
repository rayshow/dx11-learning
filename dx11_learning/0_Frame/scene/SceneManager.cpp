#include"SceneManager.h"
#include"../resource/ResourceMgr.h"

using namespace ul;

StaticMeshRender* SceneMgr::CreateStaticObject(string fileName)
{
	StaticMeshRender *pStaticObj = pResourceMgr_->CreateStaticMeshRenderFromFile(fileName);
	Null_Return_Null(pStaticObj);
	staticObjects_.push_back(pStaticObj);
	return pStaticObj;
}