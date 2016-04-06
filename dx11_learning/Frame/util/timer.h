#ifndef UL_TIMER_HEADER__
#define UL_TIMER_HEADER__

#include <windows.h>
#include <vector>
#include <cassert>

#include"tools.h"

namespace ul
{
	struct TimerFrame
	{
		ulULargeInt frameIndex_;
		ulFloat     elapase_;
	};

	class Timer
	{
	public:
		Timer(void)	:
			sampleCapacity_(60),
			index_(0),
			recordSamples_(false),
			frameElapseSecond_(0.0f),
			frameCount_(0)
		{
			QueryPerformanceFrequency(&counterFrequency_);
			QueryPerformanceCounter(&lastCount_);
			elapseTimeSamples_.resize(sampleCapacity_);
			for (unsigned int i = 0; i < sampleCapacity_; ++i)
			{
				elapseTimeSamples_[i] = new TimerFrame();
				elapseTimeSamples_[i]->frameIndex_ = 0;
				elapseTimeSamples_[i]->elapase_ = 0.0f;
			}
		}
		~Timer()
		{
			SAFE_DELETE_VECTOR<TimerFrameVector>(elapseTimeSamples_);
		}

		// Get elapsed time in seconds
		float caculate(void)
		{
			QueryPerformanceCounter(&currentCount_);

			float elapse = float((currentCount_.QuadPart - lastCount_.QuadPart) /
				double(counterFrequency_.QuadPart));
			lastCount_ = currentCount_;

			return elapse;
		}

		float GetElapsedSeconds()
		{
			return  frameElapseSecond_;
		}

		void Frame()
		{
			frameCount_++;
			frameElapseSecond_ = this->caculate();
			if (recordSamples_)
			{
				elapseTimeSamples_[index_]->frameIndex_ = frameCount_;
				elapseTimeSamples_[index_]->elapase_ = frameElapseSecond_;

				index_++;
				if ( index_ == sampleCapacity_)
				{
					index_ = 0;
				}
			}
		}

		

		float GetFPS()
		{
			//assert( std::abs(frameElapseSecond_) > 0.0001 );
			return 1.0f / frameElapseSecond_;
		}

		void SetCapacity(ulUint cap)
		{
			assert(cap >= 0);
			this->sampleCapacity_ = cap;
		}

	protected:
		typedef std::vector<TimerFrame*> TimerFrameVector;

		LARGE_INTEGER	 counterFrequency_;
		LARGE_INTEGER	 lastCount_;
		LARGE_INTEGER	 currentCount_;
		TimerFrameVector elapseTimeSamples_;
		float			 frameElapseSecond_;
		bool			 recordSamples_;
		ulUint			 sampleCapacity_;
		ulUint			 index_;
		ulUint			 frameCount_;
		
	};

};



#endif
