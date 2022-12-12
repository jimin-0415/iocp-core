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



class Wraight : public RefCountable
{
public:
	int _hp = 150;
	int _posX = 0;
	int _posY = 0;
};

using WraightRef = TSharedPtr<Wraight>;

class Missile : public RefCountable
{
public:
	//WraightRef 로 건너받게 되면 중간에 복사 생성자로 인해, 1 이하로 떨어지지 않기 때문에, 중간 개입 사항이 없어진다. 
	void SetTarget(WraightRef target) {
		_target = target;
		//누군가 중간 개입 가능.
		//target->AddRef();
		Test(target);
	}

	//복사를 할 경우 ref증가가 되니, &로 넘긴다. ref증가 안됨.
	void Test(WraightRef& temp) {	

	}

	void Update() {

		if (_target == nullptr)
			return;

		int pox = _target->_posX;
		int poy = _target->_posY;

		if (_target->_hp == 0) {
			//TSharedPtr 내부에서 알아서 객체를 관리해 줍니다.
			_target = nullptr;
		}
		//쫓아 가간다.
		
	}

	

	WraightRef _target = nullptr;
};


using MissileRef = TSharedPtr<Missile>;

int main()
{
	WraightRef wraight = new Wraight();	//생성시 1개, 복사시 1개 총 2개 세팅 되기 때문에 .Releas한번 호출
	wraight->ReleaseRef();
	MissileRef missile = new Missile();//생성시 1개, 복사시 1개 총 2개 세팅 되기 때문에 .Releas한번 호출	
	missile->ReleaseRef();
	missile->SetTarget(wraight);

	wraight->_hp = 0;
	//TSharedPtr 내부에서 알아서 객체를 관리해 줍니다.
	//wraight = WraightRef(nullptr); //nullptr 은 이거랑 같은 내용 operator=()
	wraight = nullptr;

	while (true) {
		if (missile) {
			missile->Update();
		}
	}
	
	//delete Missile
	//TSharedPtr 내부에서 알아서 객체를 관리해 줍니다.
	missile = nullptr;
}
