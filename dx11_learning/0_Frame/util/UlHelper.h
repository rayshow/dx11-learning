#ifndef UL_HELPER_HEADER__
#define UL_HELPER_HEADER__


#include<Windows.h>
#include<sstream>
#include<iostream>
#include<cstdio>
#include<string>
#include<cassert>

#include"../base/BaseDefine.h"
#include"Logger.h"


namespace ul
{
	//use in the front of enterpoint
	inline void OpenConsoleAndDebugLeak()
	{
#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//open console
		AllocConsole();
		SetConsoleTitle("debug");
		FILE* consoleFile = nullptr;
		freopen_s(&consoleFile, "CONOUT$", "wb", stdout);
#endif
		Logger::init("log.txt");
		printf("================================= log msg =======================================  ");
	}

	//set break point at leak address
	inline void SetBreakPointAtMemoryLeak(int addr)
	{
		_CrtSetBreakAlloc(addr);
	}



	//×ª»»
	inline std::wstring StringToWString(const std::string &str)
	{
		std::wstring wstr(str.length(), L' ');
		std::copy(str.begin(), str.end(), wstr.begin());
		return wstr;
	}

	inline std::string WStringToString(const std::wstring &wstr)
	{
		std::string str(wstr.length(), ' ');
		std::copy(wstr.begin(), wstr.end(), str.begin());
		return str;
	}


	template<class T>
	class AutoReleasePtr
	{
		typedef T  RefClass;
		typedef T* RefClassPtr;
	public:
		AutoReleasePtr()
		{
			refClassPtr = nullptr;
		}
		~AutoReleasePtr()
		{
			Safe_Release(refClassPtr);
		}

		RefClassPtr Get()
		{
			return refClassPtr;
		}
		RefClassPtr* GetPtr()
		{
			return &refClassPtr;
		}

		RefClassPtr operator->()
		{
			return refClassPtr;
		}

	private:
		RefClassPtr refClassPtr;
	};



	template<class T>
	class AutoDeletePtr
	{
		typedef T  RefClass;
		typedef T* RefClassPtr;
	public:
		AutoDeletePtr()
		{
			refClassPtr = nullptr;
		}
		~AutoDeletePtr()
		{
			Safe_Delete(refClassPtr);
		}

		RefClassPtr Get()
		{
			return refClassPtr;
		}
		RefClassPtr* GetPtr()
		{
			return &refClassPtr;
		}

		RefClassPtr operator->()
		{
			return refClassPtr;
		}

	private:
		RefClassPtr refClassPtr;
	};


	template<typename V>
	inline void SAFE_RELEASE_VECTOR(V &v)
	{
		if (v.size() == 0)
			return;
		typedef V::iterator I;
		I it;
		I begin = v.begin();
		I end = v.end();
		for (it = begin; it != end; ++it)
		{
			Safe_Release(*it);
		}
		v.erase(begin, end);
	}

	template<typename V>
	inline void SAFE_DELETE_VECTOR(V &v)
	{
		if (v.size() == 0)
			return;
		typedef V::iterator I;
		I it;
		I begin = v.begin();
		I end = v.end();
		for (it = begin; it != end; ++it)
		{
			Safe_Delete(*it);
		}
		v.erase(begin, end);
	}


	template<typename V>
	inline void SAFE_RELEASE_MAP(V &v)
	{
		if (v.size() == 0)
			return;
		typedef V::iterator I;
		I it;
		I begin = v.begin();
		I end = v.end();
		for (it = begin; it != end; ++it)
		{
			Safe_Release(it->second);
		}
		v.erase(begin, end);
	}

	template<typename V>
	inline void SAFE_DELETE_MAP(V &v)
	{
		if (v.size() == 0)
			return;
		typedef V::iterator I;
		I it;
		I begin = v.begin();
		I end = v.end();
		for (it = begin; it != end; ++it)
		{
			Safe_Delete(it->second);
		}
		v.erase(begin, end);
	}


};




#endif