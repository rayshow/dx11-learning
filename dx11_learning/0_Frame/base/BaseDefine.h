#ifndef UL_TOOL_HEADER__
#define UL_TOOL_HEADER__


#include<sstream>
#include<iostream>
#include<cstdio>
#include<string>
#include<cassert>



#pragma warning( disable : 4996)

namespace ul
{
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

	#ifdef _DEBUG 
		#define Ul_New new( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	#else
		#define Ul_New new
	#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

	// 调试和一些比较方便的宏
#define Null(p)  (nullptr == (p))
#define Not_Null(p) (nullptr!=(p))
#define False(p) (false == (p))
#define True(p) (true==(p))
#define Zero(p) (0==(p))
#define Fail(p) ( 0 > (p))

#define Condi_Return_Val( condi, val, msg, ...)   if (condi){ Log_Err(msg, __VA_ARGS__); assert(0); return val; }
#define Condi_Return( condi, msg, ...)   if (condi)  { Log_Err(msg, __VA_ARGS__);assert(0);  return; } 
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

	
	


};
#endif