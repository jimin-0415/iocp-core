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

class Player {
public:
	Player() {

	}
};

class Knight : public Player
{
public:
	Knight() {
		cout << "khight()" << endl;
	}

	~Knight() {
		cout << "~ khight()" << endl;
	}
	
	Knight(int32 hp)
		:_hp(hp)
	{
		cout << "Knight() hq " << endl;
	}

public:

	int32 _hp = 100;
	int32 _mp = 2000;
};

int main()
{
	for (int32 i = 0; i < 5; i++) {
		GThreadManager->Launch([]() {
			while (true) {
				Vector<Knight> v(100);

				int count = 0;
				while (true) {
					++count;
					int index = count % 99;
					cout << v[index]._hp << endl;
				}

				this_thread::sleep_for(10ms);
			}
		});
	}

	GThreadManager->Join();

	return 0;
}

