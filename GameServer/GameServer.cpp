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

int main()
{
	//Release로 할 경우 Lock을 점유 해제하는 시간이 너무 짧아서. 문제가생기지 않음.
	//Debug로 할경우
	GThreadManager->Launch([=]() {
		while (true) {
			cout << "Player Then Account" << endl;
			GPlayerManager.PlayerThenAccount();
			this_thread::sleep_for(100ms);
		}
	});
	
	GThreadManager->Launch([=]() {
		while (true) {
			cout << "Account Then Player" << endl;
			GAccountManager.AccountThenPlayer();
			this_thread::sleep_for(100ms);
		}
	});

	GThreadManager->Join();
}
