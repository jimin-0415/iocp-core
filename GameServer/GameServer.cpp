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

DECLSPEC_ALIGN(16)
class Data 
{
public:
	SLIST_ENTRY _entry;
	int64 _rand = rand() % 1000;
};

SLIST_HEADER* GHeader;

int main()
{
	GHeader = new SLIST_HEADER();
	ASSERT_CRASH(((uint64)GHeader % 16) == 0);	//메모리 16byte 정렬 여부 확인
	::InitializeSListHead(GHeader);

	for (int32 i = 0; i < 5; ++i) {
		GThreadManager->Launch([](){
			while (true) {
				Data* data = new Data();
				ASSERT_CRASH(((uint64)data % 16) == 0);

				InterlockedPushEntrySList(GHeader, (SLIST_ENTRY*)data);
				//InterlockedPushEntrySList(GHeader, (PSLIST_ENTRY)data);
				this_thread::sleep_for(10ms);
			}
			});
	}

	for (int32 i = 0; i < 2; i++) {
		GThreadManager->Launch([]() {
			while (true) {
				Data* pop = nullptr;
				pop = (Data*)InterlockedPopEntrySList(GHeader);

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

