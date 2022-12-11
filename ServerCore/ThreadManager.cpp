#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"

ThreadManager::ThreadManager()
{
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}
						
void ThreadManager::Launch(function<void(void)> callback)
{
	LockGuard	gauard(_lock);

	_threads.push_back(thread([=]() {
		InitTLS();
		callback();
		DestroyTLS();
	}));
}

void ThreadManager::Join()
{
	for (thread& t : _threads) {
		if (t.joinable())
			t.join();
	}
	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	
	//thread_local value counting
	LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{
}
