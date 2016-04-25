#ifndef UL_SCENE_MGR_HEADER__
#define UL_SCENE_MGR_HEADER__


#include"Camara.h"
#include"../design_frame/Singleton.h"
#include"../render/Environment.h"
#include"../D3D11GraphicContext.h"

namespace ul
{
	class SceneMgr :public Singleton<SceneMgr>
	{
	private:
		Environment  			   mainEnvirment_;
		SkyBox					   mainSkyBox_;
		BaseCamara*				   mainCamara_;
		vector<BaseCamara*>        camaras_;
		vector<StaticMeshRender*>  staticObjects_;
		ResourceMgr*               pResourceMgr_;
	public:
		SceneMgr(){};
		~SceneMgr(){};
			

		bool Initialize(ResourceMgr* mgr)
		{
			pResourceMgr_ = mgr;
			False_Return_False( mainSkyBox_.Create() );
		}

		void Shutdown()
		{
			for (ulUint i = 0; i < camaras_.size(); ++i)
			{
				Safe_Delete(camaras_.at(i));
			}
		}

	public:

		StaticMeshRender* CreateStaticObject(string fileName);

		BaseCamara* CreateCamara()
		{
			BaseCamara* camara = new BaseCamara();
			camaras_.push_back(camara);
			return camara;
		}

		void SetMainCamara(BaseCamara* camara)
		{
			mainCamara_ = camara;
		}

		void SetEnvironment(string envFileName)
		{
			mainEnvirment_.Initialize(envFileName);
		}

		Environment& GetMainEnvironmentRef()
		{
			return mainEnvirment_;
		}

		BaseCamara* GetMainCamaraPtr()
		{
			return mainCamara_;
		}

		void RenderAll(ID3D11DeviceContext* context)
		{
			D3D11GraphicsContext::GetSingletonPtr()->DisableDepthTest();
			mainSkyBox_.Render(context);
			for (ulUint i = 0; i < staticObjects_.size(); ++i)
			{
				staticObjects_.at(i)->Render(context);
			}
		}

	};
};








#endif