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

class Knight {
public:
	Knight() {
		cout << "khight()" << endl;
	}

	~Knight() {
		cout << "~ khight()" << endl;
	}

public:
private:
	int32 _hp = 100;
	int32 _mp = 2000;
};

int main()
{
	//new : 메모리 할당 -> 생성자 호출
	//delete : 소멸자 호출 -> 메모리 날림 

	//Knight* a = new Knight();
	Knight* a = xnew<Knight>();
	//delete a;
	xdelete(a);
	
	return 0;
}

//사용자 정의 new, delete, 해당 프로젝트의 정책에 맞춘 메모리 관리가 가능하다.

/*
class Knight {
public:
	Knight() {
		cout << "khight()" << endl;
	}

	~Knight() {
		cout << "~ khight()" << endl;
	}

public:
	//static 함수이기 때문에 붙이 static 키워드 붙여 줄 필요 없다.
	static void* operator new(size_t size) {
		cout << "new !!!!! " << size << endl;
		void* ptr = ::malloc(size);
		return ptr;
	}

	//static 함수이기 때문에 붙이 static 키워드 붙여 줄 필요 없다.
	static void operator delete(void* ptr) {
		cout << "delete !!!!! " << endl;
		::free(ptr);
	}

private:
	int32 _hp = 100;
	int32 _mp = 2000;
};
*/