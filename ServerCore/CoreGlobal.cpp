#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"

ThreadManager* GThreadManager;

// Core Global 을 cpp안에 정의해서 전역 객체로 사용한다.
class CoreGlobal	
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
	}

	~CoreGlobal()
	{
		delete GThreadManager;
	}
} GCoreGlobal;
