#ifndef SINGLETON_HEADER__
#define SINGLETON_HEADER__

#include<cassert>
namespace ul{

	template<class T>
	class Singleton
	{
	private:
		Singleton(const Singleton<T> &);
		Singleton& operator=(const Singleton<T> &);

	public:
		Singleton()
		{

			if (s_Singleton)
				return;
#if defined( _MSC_VER ) && _MSC_VER < 1200	 
			int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
			s_Singleton = (T*)((int)this + offset);
#else
			s_Singleton = static_cast< T* >(this);
#endif
			int a =0;
		}
		static T* GetSingletonPtr()
		{
			assert(nullptr != s_Singleton);
			return s_Singleton;
		}

		static T& GetSingleton()
		{
			assert(nullptr != s_Singleton);
			return *s_Singleton;
		}
	public:
		static T *s_Singleton;
	};

	template<class T> T* Singleton<T>::s_Singleton = nullptr;

};





#endif