#ifndef UL_SINGLETON_HEADER__
#define UL_SINGLETON_HEADER__

#include<cassert>
namespace ul{

	template<class T>
	class Singletonable
	{
	private:
		Singletonable(const Singletonable<T> &);
		Singletonable& operator=(const Singletonable<T> &);

	public:
		Singletonable()
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

	template<class T> T* Singletonable<T>::s_Singleton = nullptr;

};





#endif