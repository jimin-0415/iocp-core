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

class Knight
{
public :
	int32 _hp = rand() % 100;
};

int main()
{
	for (int32 i = 0; i < 5; ++i) {
		GThreadManager->Launch([]() {
			while (true) {
				Knight* a = xnew<Knight>();
				cout << a->_hp << endl;

				this_thread::sleep_for(10ms);

				xdelete(a);
			}
			});
	}
	return 0;
}

