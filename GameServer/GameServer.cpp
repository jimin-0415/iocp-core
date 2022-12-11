#include "pch.h"
#include <windows.h>
#include <future>
#include <mutex>
#include <thread> 
#include <atomic>
#include <chrono>
#include "ThreadManager.h"

CoreGlobal core;

void ThreadMain() {
	while (true) {
		cout << "Hell I AM Thread .." << LThreadId << endl;
		this_thread::sleep_for(1s);
	}
}

int main()
{
	for (int32 i = 0; i < 5; ++i) {
		GThreadManager->Launch(ThreadMain);
	}
	GThreadManager->Join();
}
