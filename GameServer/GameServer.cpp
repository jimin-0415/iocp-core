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
	Knight* test = new Knight; //먼가 내부에서 재사용성 효율을 높이는거 같다.
	test->_hp = 100;
	delete test;
	test->_hp = 200;

	Knight* knight = xnew<Knight>(100);
	xdelete(knight);
	
	//delete 후 잘못된 메모리 접근 잡아줌.
	knight->_mp =  1000;

	//현재 문제점 메모리 오버플로우 문제는 못잡음
	//[[  할당 크기] [넘어간크기]             ] <- 문제되는 부분 메모리 오버플로우 발생
	Knight* knight2 = (Knight*)xnew<Player>();
	knight2->_hp = 200; //해당 구문은 현재 정상적으로 돌아갑니다. <- 이렇게 하면 잡힘

	//해결방법 
	//	메모리를 끝으로 보내버린다.
	//[         4kb        [ 할당 공간 ]]
	// 이유 -> 대부분 메모리 오버플로우가 문제가 되기 때문에
	//메모리 언더 플로우는 거의 발생하지 않는다.
	return 0;
}

