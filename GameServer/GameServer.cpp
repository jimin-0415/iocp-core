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

class Missile : public RefCountable
{
public:
	void SetTarget(Wraight* target) {
		_target = target;
		target->AddRef();
	}

	void Update() {

		if (_target == nullptr)
			return;

		int pox = _target->_posX;
		int poy = _target->_posY;

		if (_target->_hp == 0) {
			_target->ReleaseRef();
			_target = nullptr;
		}

		//쫓아 가간다.
	}

	Wraight* _target = nullptr;
};


int main()
{
	Wraight* wraight = new Wraight();
		
	Missile* missile = new Missile();
	missile->SetTarget(wraight);

	//여기서 wraight 를 삭제 했으면 잘못된 동작을 합니다, 오염된 코드에 접근함.
	wraight->_hp = 0;
	wraight->ReleaseRef();
	wraight = nullptr;

	while (true) {
		if (missile) {
			missile->Update();
		}
	}
	
	//delete Missile
	missile->ReleaseRef();
	missile = nullptr;
}
