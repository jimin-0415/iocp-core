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



using KnightRef = TSharedPtr<class Knight>;
using InventoryRef = TSharedPtr<class Inventory>;

class Knight : public RefCountable 
{
public:
	Knight() {
		cout << "khight()" << endl;
	}

	~Knight() {
		cout << "~ khight()" << endl;
	}

public:
	void SetTarget(KnightRef target) {
		_target = target;
	}

	void SetInventory(InventoryRef& inventory) {
		_inventory = inventory;
	}

private:
	KnightRef _target;
	InventoryRef _inventory = nullptr;
};



class Inventory : public RefCountable
{
public:
	Inventory(KnightRef knight) 
		:_target(**knight) //ref에서 역참조 
	{
	}

private:
	Knight& _target; 
};

int main()
{
	KnightRef k1(new Knight());
	k1->ReleaseRef();
	KnightRef k2(new Knight());
	k2->ReleaseRef();

	//서로 주시하고 있기 때문에 순환 참조를 하게 된다. Thread의 순환 참조와 같음
	//컴포넌트 패턴처럼, 다른 클래스를 참조하고 있을 경우 발생한다.
	k1->SetTarget(k2);
	k2->SetTarget(k1);

	//해결 1
	k1->SetTarget(nullptr);
	k2->SetTarget(nullptr);

	k1 = nullptr;
	k2 = nullptr;


	KnightRef k3(new Knight());
	k3->SetInventory(*new InventoryRef());
	/*
	* 해당 상황은 문제가 되지 않습니다.
	* class Inventory : public RefCountable
	{
		public:
			Inventory(KnightRef knight)
				:_target(**knight) //ref에서 역참조
			{
			}

		private:
			Knight& _target; <- KnightRef& _target; 으로 참조할 경우 SmartPointer 순환 참조 현상이 발생함
	};
	*/
	//순환 참조를 할 경우, 메모리가 해지 되지 않기 때문에 메모리 사용량이 늘어 메모리 Leek이 발생합니다.
	//순환 참조는 표준 SmartPointer도 동일하다. 순환 참조는..

	//1. Unqiue Ptr
	// 1.스마트 포인터끼리 복사가 불가능하다.
	// 2.복사가 불가능하기 때문에 이동을 시켜야 한다.
	// 3.객체가 자동 소멸된다.
	unique_ptr<Knight> k4 = make_unique<Knight>();
	//unique_ptr<Knight> k3 = k2; [x]
	unique_ptr<Knight> k5 = std::move(k4);


	//3. Shared Ptr
	// ref를 상속받지 않은 객체에도 사용가능하다.
	// 직접 만든 ref방식과 차이가 있다.
	// Ref Count 가 상속 관계가 아니구, has a 관계로 구성되어있다.
	//[ref][RefCountingBlock]
	shared_ptr<Knight>spr3 = make_shared<Knight>(); //한번에 만들어 버린다.
	shared_ptr<Knight> spr4 (new Knight());			//make_shared 란 생성자를 통해서 만드는 부분하고 차이가 있다.
	
	//make_shared: 미리 할당할 공간을 잡은 후 한번에 만든다.
	//(new Knight)로 생성할 경우 knight 를 할 당 후 ref 공간을 할당한다.
	//RefCOuntBlock 에서는 _Use카운트와 Week 카운트를 두개의 변수를 가지고 있다.

	shared_ptr<Knight> spr;
	shared_ptr<Knight>spr2 = spr;

	//2. Week Ptr
	// WeekPtr은 RefCountBlock을 참조해서 해당 객체가 진짜로 사라졌는지 아니면 사라지지 않았는지 알 수 있지만, 관리객체의 수명에는 관여하지 않는다.
	// 사이클 문제를 해결할 수 있다. 
	weak_ptr<Knight> wek1 = spr2;
	bool expire = wek1.expired(); //weekpointer 만료 확인.
	shared_ptr<Knight> sp2 = wek1.lock();	//lock을 통해서 shared_ptr로 변환해서 사용하는 방식.
	// 만약 해당 객체가 모두 해제되면 lock() 을 하면 null이 됨.

	//RefCountBlock 
	//	(useCount) : Shared
	//	(weekCount) : week
	// -> useCount 가 0이면 관리하는 객체는 날라간다. 하지만 RefCountBlock은 소멸되지 않는다.
	// -> weekCount가 

	//week_ptr : <- shared만 사용할 수 있고, week만 사용할 수 있다.
	// 관리 객체에 대한 수명 주기에는 영향을 주지 않는다. 
	//막상 week_ptr을 사용하면, 두번에 걸쳐서 작업을 해야되서 번거롭다. <- 프로젝트에 따라서 선택의 영역이다.
	
	return 0;
}
