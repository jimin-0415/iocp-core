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

#include "TypeCast.h"

enum PLAYER_TYPE {
	KNIGHT,
	MONSTER,
};

class Player 
{
public:
	Player() {
	}
	virtual ~Player();

	int _type;
};

class Knight : public Player
{
public:
	Knight() {
		_type = KNIGHT;
	}
public :
	int32 _hp = rand() % 100;
};

class Monster : public Player
{
public:
	Monster() {
		_type = MONSTER;
	}
public:
	int64 _id = 0;
};

class A {

};


int main()
{
	//대부분 프로젝트는 상위 객체로 관리하다가
	//다시 다운 캐스팅을 해야한다.
	Player* p1 = new Knight();
	Player* p2 = new Monster();

	Knight* k1 = static_cast<Knight*>(p2);
	Knight* k2 = dynamic_cast<Knight*>(p2);  

	int32 len1 = Length<TypeList<Player, Monster, Knight>>::value;

	using TL = TypeList<Player, Monster, Knight>;
	TypeAt<TL, 0>::Result typeAt0; //Player
	TypeAt<TL, 1>::Result typeAt1; //Monster
	TypeAt<TL, 2>::Result typeAt2; //Knight

	int index = IndexOf<TL, Player>::value;
	int index2 = IndexOf<TL, Monster>::value;
	int index3 = IndexOf<TL, Knight>::value;
	int index4 = IndexOf<TL, A>::value;		//-1 못찾음

	//Static Cast 는 위험다.
	//Dynamic Cast는 안전하지만, 성능상 좋지 않다.

	//1. 성능 개선 -> enum을 두고 enum check후 static_cast 를 진행하낟.
	if(k1->_type == KNIGHT)
		Knight* k1 = static_cast<Knight*>(p2);

	//1-1. 문제점. -> 클래스가 많아지면 일일이 코드 만드는게 일이다.
	//1-2. 체크 누락시 문제가 발생
	//1-3. 여러작업자중 누군가가 당연히 캐스팅 해도 되겠지 하는 경우도 발생.

	//2. 함수를 만들어서 관리한다. -> 이것도 일일이 만들어야 한다.



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

