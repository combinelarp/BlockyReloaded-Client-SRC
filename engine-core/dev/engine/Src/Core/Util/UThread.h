#ifndef __ENGINE_UTHREAD_HEADER__
#define __ENGINE_UTHREAD_HEADER__

#include "Core.h"

namespace LORD
{
	enum class ThreadPriority
	{
		LOW,
		NORMAL,
		HIGH
	};

	class UThread
	{
	public:
		UThread() = delete;

		static void setThisThreadName(const char* name);
		static void setThisThreadPriority(ThreadPriority priority);
		static int getThisThreadNativeId();
	};
}

#endif