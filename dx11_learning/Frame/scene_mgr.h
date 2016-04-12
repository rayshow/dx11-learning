#ifndef UL_SCENE_MGR_HEADER__
#define UL_SCENE_MGR_HEADER__


#include"util/environmentable.h"
#include"util/singletonable.h"
#include"camara.h"


namespace ul
{
	class SceneMgr :public Singletonable<SceneMgr>
	{
	private:
		BaseCamara*		 mainCamara_;
		Environmentable* curEnvirment_;
	public:
		SceneMgr()
		{
			mainCamara_ = new BaseCamara();
			mainCamara_->LookAt(XMFLOAT4(0, 0, -1, 0), XMFLOAT4(0, 0, 0, 0));
			mainCamara_->SetProject(BaseCamara::eCamara_Perspective, XM_PI / 4, 16.0/9.0, 0.1f, 1000.0f);
			curEnvirment_ = new Environmentable();
		}

		~SceneMgr()
		{
			Safe_Delete(mainCamara_);
			Safe_Delete(curEnvirment_);
		}
	public:
		void SetCamara(BaseCamara* camara)
		{
			mainCamara_ = camara;
		}

		void SetEnvironment(Environmentable* env)
		{
			curEnvirment_ = env;
		}

		BaseCamara* GetMainCamara()
		{
			return mainCamara_;
		}

		Environmentable* GetEnvironment()
		{
			return curEnvirment_;
		}
	};
};








#endif