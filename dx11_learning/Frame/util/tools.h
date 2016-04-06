#ifndef UL_TOOL_HEADER__
#define UL_TOOL_HEADER__

#include<Windows.h>
#include<sstream>
#include<iostream>
#include<cstdio>
#include<string>
#include<cassert>

#include"logger.h"

using std::string;
using std::wstring;

#pragma warning( disable : 4996)

namespace ul
{
	///////////////////////////////////////////////////////////////////////////////////////
	// 内存泄漏检测支持
	#ifdef _DEBUG 
		#define Ul_New new( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	#else
		#define Ul_New new
	#endif

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
				printf("================= debug msg ==================== \n ");
	}

	//set break point at leak address
	inline void SetBreakPointAtMemoryLeak(int addr)
	{
		_CrtSetBreakAlloc(addr);
	}
	////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////
	// 基本类型统一
#ifdef UNICODE
#define String  wstring
#else 
#define ulString string
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

	typedef unsigned int  ulUint;
	typedef unsigned char ulUbyte;
	typedef unsigned long ulUlong;
	typedef unsigned short ulUshort;
	typedef int ulInt;
	typedef long ulLong;
	typedef char ulByte;
	typedef short ulShort;
	typedef float ulFloat;
	typedef double ulDFloat;
	typedef bool   ulBit;
	typedef unsigned long long ulULargeInt;
	////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////
	// 调试和一些比较方便的宏
#define Null(p)  (nullptr == (p))
#define False(p) (false == (p))
#define Zero(p) (0==(p))
#define Fail(p) ( 0 > (p))

#define Log_Err(msg, ...)    Logger::error(__FILE__, __LINE__, msg,  __VA_ARGS__)
#define Log_Info(msg, ...)   Logger::info( msg,   __VA_ARGS__)
#define Log_Warn(msg,  ...)  Logger::warn(__FILE__,  __LINE__, msg,  __VA_ARGS__)
#define Log_Fatal(msg, ...)  Logger::fatal(__FILE__, __LINE__, msg,  __VA_ARGS__)


#define Condi_Return_Val( condi, val, msg, ...)   \
	if (condi)  \
	{ \
		Log_Err(msg, __VA_ARGS__); \
		assert(0); \
		return val; \
	}

#define Condi_Return( condi, msg, ...)   \
	if (condi)  \
	{ \
		Log_Err(msg, __VA_ARGS__); \
		assert(0); \
		return; \
	} 

#define Null_Return_Null(ptr)  Condi_Return_Val((nullptr == ptr), (nullptr), "pointer is nullptr.")
#define Null_Return_Void(ptr)  Condi_Return(    (nullptr == ptr),            "pointer is nullptr.")
#define Null_Return_Fail(ptr)  Condi_Return_Val((nullptr == ptr), (-1),      "pointer is nullptr.")
#define Null_Return_False(ptr) Condi_Return_Val((nullptr == ptr), (false),   "pointer is nullptr.")
#define Null_Return_Null_With_Msg(ptr, msg, ...)  Condi_Return_Val((nullptr == ptr), (nullptr), msg, __VA_ARGS__)
#define Null_Return_Void_With_Msg(ptr, msg, ...)  Condi_Return(    (nullptr == ptr),            msg, __VA_ARGS__)
#define Null_Return_Fail_With_Msg(ptr, msg, ...)  Condi_Return_Val((nullptr == ptr), (-1),      msg, __VA_ARGS__)
#define Null_Return_False_With_Msg(ptr, msg, ...) Condi_Return_Val((nullptr == ptr), (false),   msg, __VA_ARGS__)

#define False_Return_Null(c)  Condi_Return_Val((false == c), (nullptr), "condition false.")
#define False_Return_Void(c)  Condi_Return(    (false == c),            "condition false.")
#define False_Return_Fail(c)  Condi_Return_Val((false == c), (-1),      "condition false.")
#define False_Return_False(c) Condi_Return_Val((false == c), (false),   "condition false.")
#define False_Return_Null_With_Msg(c, msg, ...)  Condi_Return_Val((false == c), (nullptr), msg, __VA_ARGS__)
#define False_Return_Void_With_Msg(c, msg, ...)  Condi_Return(    (false == c),            msg, __VA_ARGS__)
#define False_Return_Fail_With_Msg(c, msg, ...)  Condi_Return_Val((false == c), (-1),      msg, __VA_ARGS__)
#define False_Return_False_With_Msg(c, msg, ...) Condi_Return_Val((false == c), (false),   msg, __VA_ARGS__)

#define Fail_Return_Null(h)  Condi_Return_Val((0 > h), (nullptr), "result failed.")
#define Fail_Return_Void(h)  Condi_Return(    (0 > h),            "result failed.")
#define Fail_Return_Fail(h)  Condi_Return_Val((0 > h), (-1),      "result failed.")
#define Fail_Return_False(h) Condi_Return_Val((0 > h), (false),   "result failed.")
#define Fail_Return_Null_With_Msg(h, msg, ...)  Condi_Return_Val((0 > h), (nullptr), msg, __VA_ARGS__)
#define Fail_Return_Void_With_Msg(h, msg, ...)  Condi_Return(    (0 > h),            msg, __VA_ARGS__)
#define Fail_Return_Fail_With_Msg(h, msg, ...)  Condi_Return_Val((0 > h), (-1),      msg, __VA_ARGS__)
#define Fail_Return_False_With_Msg(h, msg, ...) Condi_Return_Val((0 > h), (false),   msg, __VA_ARGS__)



#ifndef Array_Size
#define Array_Size(arr) ( sizeof(arr)/sizeof(arr[0]) )
#endif

#ifndef Safe_Delete
#define Safe_Delete(p) if(nullptr!=p) { delete p; p=nullptr; }
#endif
#ifndef Safe_Delete_Array
#define Safe_Delete_Array(p) if(nullptr!=p){ delete[] p; p=nullptr; }
#endif

#ifndef Safe_Release
#define Safe_Release(p)      { if (p) { (p)->Release(); (p)=nullptr; } }
#endif

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

	/////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//有用的函数
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

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	


};
#endif