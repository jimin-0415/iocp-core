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
#include "ObjectPool.h"

class Knight
{
public :
	int32 _hp = rand() % 100;
};

class Monster
{
public:
	int64 _id = 0;
};

int main()
{
	//Object Pool 수동 관리 부분.
	Knight* k =  ObjectPool<Knight>::Pop();
	ObjectPool<Knight>::Push(k);

	Knight* knight[100];

	for (int32 i = 0; i < 100; ++i) {
		knight[i] = ObjectPool<Knight>::Pop();
	}

	for (int32 i = 0; i < 100; ++i) {
		ObjectPool<Knight>::Push(knight[i]);
	}
	
	//ObjectPool Knight 생성
	shared_ptr<Knight> sptr = ObjectPool<Knight>::MakeShared();
	
	//Memory Pool Knight 생성
	shared_ptr<Knight> sptr2 = MakeShared<Knight>();

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

