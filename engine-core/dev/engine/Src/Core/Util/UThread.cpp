#define _GNU_SOURCE
#include "Core.h"
#include "UThread.h"

#if LORD_PLATFORM == LORD_PLATFORM_WINDOWS
#include <windows.h>
#elif LORD_PLATFORM == LORD_PLATFORM_LINUX || LORD_PLATFORM == LORD_PLATFORM_ANDROID
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>

#if LORD_PLATFORM != LORD_PLATFORM_ANDROID
#define gettid() syscall(SYS_gettid)
#endif
#endif

namespace LORD
{
#if LORD_PLATFORM == LORD_PLATFORM_WINDOWS
void UThread::setThisThreadName(const char* name)
{
	#pragma pack(push,8)
	struct tagTHREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	} info;
	#pragma pack(pop)
	info.dwType = 0x1000;
	info.szName = name;
	info.dwThreadID = -1;
	info.dwFlags = 0;
	RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
}

void UThread::setThisThreadPriority(ThreadPriority priority)
{
	int windowsPriority;
	switch (priority)
	{
	case ThreadPriority::LOW:
		windowsPriority = THREAD_PRIORITY_LOWEST;
		break;
	case ThreadPriority::NORMAL:
		windowsPriority = THREAD_PRIORITY_NORMAL;
		break;
	case ThreadPriority::HIGH:
		windowsPriority = THREAD_PRIORITY_HIGHEST;
		break;
	default:
		LordLogError("unsupported priority value %d", priority);
		return;
	}
	SetThreadPriority(GetCurrentThread(), windowsPriority);
}

int UThread::getThisThreadNativeId()
{
	return GetThreadId(GetCurrentThread());
}

#elif LORD_PLATFORM == LORD_PLATFORM_LINUX || LORD_PLATFORM == LORD_PLATFORM_ANDROID
void UThread::setThisThreadName(const char* name)
{
#if LORD_PLATFORM != LORD_PLATFORM_MAC_IOS
	pthread_setname_np(pthread_self(), name);
#endif
}

void UThread::setThisThreadPriority(ThreadPriority priority)
{
	int niceValue;
	switch (priority)
	{
	case ThreadPriority::LOW:
		niceValue = 2;
		break;
	case ThreadPriority::NORMAL:
		niceValue = 0;
		break;
	case ThreadPriority::HIGH:
		niceValue = -2;
		break;
	default:
		LordLogError("unsupported priority value %d", priority);
		return;
	}
	if (setpriority(PRIO_PROCESS, gettid(), niceValue) != 0)
	{
		LordLogError("Failed to set thread nice value: %s", strerror(errno));
	}
}

int UThread::getThisThreadNativeId()
{
	return gettid();
}

#else
void UThread::setThisThreadName(const char* name)
{
	(void)name;
}

void UThread::setThisThreadPriority(ThreadPriority priority)
{
	(void)priority;
}

int UThread::getThisThreadNativeId()
{
	return 0;
}

#endif
}