#ifndef UL_TIMER_HEADER__
#define UL_TIMER_HEADER__

#include <windows.h>
#include <vector>
#include <cassert>
#include"UlHelper.h"

namespace ul
{
	class Timer
	{
	public:
		Timer(void)	
		{
			QueryPerformanceFrequency(&counterFrequency_);
			QueryPerformanceCounter(&lastCount_);
			secondPerCount_ = 1.0 / counterFrequency_.QuadPart;
		}
		~Timer()
		{
			
		}

		float GetDeltaTime()
		{
			return  elapseSecond_;
		}

		void Tick()
		{
			QueryPerformanceCounter(&currentCount_);
			elapseSecond_ = (currentCount_.QuadPart - lastCount_.QuadPart) * secondPerCount_;
			lastCount_ = currentCount_;

			time_ += elapseSecond_;

		}

		float GetFPS()
		{
			if (time_ > 1.0)
			{
				fps_ = 1.0 / elapseSecond_;
				time_ = 0.0;
				return fps_;
			}
			return fps_;
		}

	
	protected:
		LARGE_INTEGER	 counterFrequency_;
		LARGE_INTEGER	 lastCount_;
		LARGE_INTEGER	 currentCount_;
		double           secondPerCount_;
		double           elapseSecond_;
		double           fps_;
		double           time_;
	};

};



#endif
