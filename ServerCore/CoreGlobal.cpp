#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"
#include "Memory.h"
#include "SocketUtils.h"
ThreadManager* GThreadManager = nullptr;
DeadLockProfiler* GDeadLockProfiler = nullptr;
Memory* GMemory = nullptr;

// Core Global 을 cpp안에 정의해서 전역 객체로 사용한다.
class CoreGlobal	
{
public:
	CoreGlobal()
	{
		GMemory = new Memory();
		GThreadManager = new ThreadManager();
		GDeadLockProfiler = new DeadLockProfiler();
		SocketUtils::Init();
	}

	~CoreGlobal()
	{
		delete GThreadManager;
		delete GDeadLockProfiler;
		delete GMemory;
		SocketUtils::Clear();
	}
} GCoreGlobal;
