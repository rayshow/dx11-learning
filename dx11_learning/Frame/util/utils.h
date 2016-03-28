#ifndef APPLICATION_TOOLS_H__
#define APPLICATION_TOOLS_H__


#include<Windows.h>
#include<sstream>
#include<iostream>
#include<cstdio>
#include<string>
#include<cassert>

#include"logger.h"

#ifdef _DEBUG
#pragma comment(lib, "loggerd-v120.lib")
#else
#pragma comment(lib, "logger-v120.lib")
#endif

using std::string;
using std::wstring;

#pragma warning( disable : 4996)
namespace Utils
{
	///////////////////////////////////////////////////////////////////////////////////////
	// 内存泄漏检测支持
	#ifdef _DEBUG 
		#define debug_new new( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	#else
		#define debug_new new
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

		#else
				ul::Logger::init("log.txt");
		#endif
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
		#define String string
	#endif

	#ifdef _MSC_VER
	#define snprintf _snprintf
	#endif

	#ifndef uint
	#define uint unsigned int
	#endif
	#ifndef uchar
	#define uchar unsigned char
	#endif
	#ifndef ushort
	#define ushort unsigned short
	#endif
	#ifndef ulong
	#define ulong unsigned long
	#endif
	////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////
	// 调试和一些比较方便的宏
	#define IS_NULL(p)  (NULL == (p))

#define LOG_EXCEPTION(msg, fuc) {  \
	char buf[1024]; memset(buf, 0, 1024); \
	snprintf(buf, 1024, " ERROR : %s file: %s line: %d ", msg, __FILE__, __LINE__ ); \
	fuc(buf); \
	}



#define LOG_ERR(msg)  LOG_EXCEPTION(msg, Logger::error)
#define LOG_WARN(msg) LOG_EXCEPTION(msg, Logger::warn)
#define LOG_FATAL(msg) LOG_EXCEPTION(msg, Logger::fatal)
#define LOG_INFO(msg) Logger::info(msg)

#define CONDITION_RETURN_X_LOG(condi, msg, x, log ) \
	{\
	if (condi){ log(msg); assert(0);   return x; } \
	}

#define CONDITION_RETURN_X_ERR(condi, msg, x )  CONDITION_RETURN_X_LOG(condi, msg, x, LOG_ERR )
#define CONDITION_RETURN_X_WARN(condi, msg, x )  CONDITION_RETURN_X_LOG(condi, msg, x, LOG_WARN )
#define CONDITION_RETURN_X_FATAL(condi, msg, x )  CONDITION_RETURN_X_LOG(condi, msg, x, LOG_FATAL )
#define CONDITION_RETURN_X_INFO(condi, msg, x )  CONDITION_RETURN_X_LOG(condi, msg, x, LOG_INFO )


#define NULL_RETURN_X_MSG(p, msg, x, ret)  CONDITION_RETURN_X_ERR( (nullptr==p), msg,  x)
#define NULL_RETURN_X(p, x)           NULL_RETURN_X_MSG(p, "pointer is null.", x)
#define FAIL_RETURN_X_MSG(p, msg, x)  CONDITION_RETURN_X_ERR( ( FAILED(p) ), msg,  x)
#define FAIL_RETURN_X(p, x)			  FAIL_RETURN_X_MSG(p, "result is exceptional.", x)
#define FALSE_RETURN_X_MSG(p, msg, x) CONDITION_RETURN_X_WARN( ( !p ), msg,  x)
#define FALSE_RETURN_X(p, x)		  FALSE_RETURN_X_MSG(p, "get a false.", x)

#define RETURN_VALUE { return void;}
#define RETURN_VOID RETURN_VALUE
	//judge result if nullptr and return x
#define NULL_RETURN_NULL(p)			   NULL_RETURN_X(p, nullptr)
#define NULL_RETURN_VOID(p)			   NULL_RETURN_X(p, void)
#define NULL_RETURN_FALSE(p)		   NULL_RETURN_X(p, false)
#define NULL_RETURN_NULL_MSG(p, msg)   NULL_RETURN_X_MSG(p, msg, nullptr)
#define NULL_RETURN_VOID_MSG(p, msg)   NULL_RETURN_X_MSG(p, msg, void)
#define NULL_RETURN_FALSE_MSG(p, msg)  NULL_RETURN_X_MSG(p, msg, false)

	//judge result is false and return x
#define FALSE_RETURN_NULL(p)		   FALSE_RETURN_X(p, nullptr)
#define FALSE_RETURN_VOID(p)		   FALSE_RETURN_X(p, void)
#define FALSE_RETURN_FALSE(p)		   FALSE_RETURN_X(p, false)
#define FALSE_RETURN_NULL_MSG(p, msg)  FALSE_RETURN_X_MSG(p, msg, nullptr)
#define FALSE_RETURN_VOID_MSG(p, msg)  FALSE_RETURN_X_MSG(p, msg, void)
#define FALSE_RETURN_FALSE_MSG(p,msg)  FALSE_RETURN_X_MSG(p, msg, false)

	//judge result if FAILED and return x
#define FAIL_RETURN_NULL(p)			   FAIL_RETURN_X(p, nullptr)
#define FAIL_RETURN_VOID(p)			   FAIL_RETURN_X(p, void)
#define FAIL_RETURN_FALSE(p)		   FAIL_RETURN_X(p, false)
#define FAIL_RETURN_NULL_MSG(p, msg)   FAIL_RETURN_X_MSG(p, msg, nullptr)
#define FAIL_RETURN_VOID_MSG(p, msg)   FAIL_RETURN_X_MSG(p, msg, void)
#define FAIL_RETURN_FALSE_MSG(p,msg)   FAIL_RETURN_X_MSG(p, msg, false)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) ( sizeof(arr)/sizeof(arr[0]) )
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(nullptr!=p) { delete p; p=nullptr; }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(nullptr!=p){ delete[] p; p=nullptr; }
#endif

#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

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
			SAFE_RELEASE(*it);
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
			SAFE_RELEASE(it->second);
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