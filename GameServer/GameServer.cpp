#include "pch.h"
#include <windows.h>
#include <future>
#include <mutex>
#include <thread> 
#include <atomic>
#include <chrono>
#include "ThreadManager.h"

class TestLock {
	USE_LOCK;

public:
	int32 TestRead() {
		READ_LOCK;

		if (_queue.empty())
			return -1;

		return _queue.front();
	}

	void TestPush() {
		WRITE_LOCK;

		_queue.push(rand() % 100);
	}

	void TestPop() {
		WRITE_LOCK;

		if (_queue.empty() == false)
			_queue.pop();
	}

private:
	queue<int32> _queue;
};

TestLock testLock;

void ThreadWrite() {
	while (true) {
		testLock.TestPush();
		this_thread::sleep_for(1ms);
		testLock.TestPop();
	}
}

void ThreadRead() {
	while (true) {
		int32 val = testLock.TestRead();
		cout << val << endl;
		this_thread::sleep_for(1ms);
	}
}

int main()
{
	for (int32 i = 0; i < 3; ++i) {
		GThreadManager->Launch(ThreadWrite);
	}
	for (int32 i = 0; i < 5; ++i) {
		GThreadManager->Launch(ThreadRead);
	}

	GThreadManager->Join();
}
