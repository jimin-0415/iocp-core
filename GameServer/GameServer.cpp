#include "pch.h"
#include <windows.h>
#include <future>
#include <mutex>
#include <thread> 
#include <atomic>
#include <chrono>
#include "ThreadManager.h"
#include "PlayerManager.h"
#include "AccountManager.h"

#include "RefCounting.h"
#include "Memory.h"
#include "LockFreeStack.h"

SListHeader* GHeader;

int main()
{
	GHeader = new SListHeader();
	ASSERT_CRASH(((uint64)GHeader % 16) == 0);	//메모리 16byte 정렬 여부 확인
	InitializeHeader(GHeader);

	for (int32 i = 0; i < 5; ++i) {
		GThreadManager->Launch([](){
			while (true) {
				Data* data = new Data();
				ASSERT_CRASH(((uint64)data % 16) == 0);

				PushEntryList(GHeader, (SListEntry*)data);
				this_thread::sleep_for(10ms);
			}
			});
	}

	for (int32 i = 0; i < 2; i++) {
		GThreadManager->Launch([]() {
			while (true) {
				Data* pop = nullptr;
				pop = (Data*)PopEntryList(GHeader);

				if (pop) {
					cout << pop->_rand << endl;
					delete pop;
				}
				else {
					cout << "NONE" << endl;
				}
			}
		});
	}
	return 0;
}

