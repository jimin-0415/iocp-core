#pragma once
#include "Types.h"

/// <summary>
/// RW SpinLock
/// 
/// [16bit(W)][16bit(R)] 의 의미가 달라짐
/// W : WriteFlag (Exclusive Lock Owner ThreadId) <- Lock을 획득한 ThreadID
/// R : ReadFlag (Shared Lock Count)
/// 
/// </summary>
class Lock
{
	enum : uint32 
	{
		ACQUIRE_TIMEOUT_TICK = 10000, //10ms
		MAX_SPIN_COUNT = 5000,

		EMPTY_FLAG =		0x0000'0000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_COUNT_MASK =	0x0000'FFFF,
	};

public:
	void WriteLock();
	void WriteUnlock();
	void ReadLock();
	void ReadUnlock();

private:
	Atomic<uint32> _lockFlag = EMPTY_FLAG;
	uint16 _writeCount = 0;
};

class ReadLockGuard 
{
public:
	ReadLockGuard(Lock & lock)
		:_lock(lock){
		_lock.ReadLock();
	}
	~ReadLockGuard() {
		_lock.ReadUnlock();
	}
private:
	Lock& _lock;
};


class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock)
		:_lock(lock) {
		_lock.WriteLock();
	}
	~WriteLockGuard() {
		_lock.WriteUnlock();
	}
private:
	Lock& _lock;
};

///
/// RwLock을 만드는 이유
/// 1. Mutex의 경우는 어떤 문제가 있나?
///  
/// 1. 표준 Mutex의 경우는 제귀적으로 Lock을 잡을 수 없다.
/// m.lock()을 잡은 상태에서 또다른 내부 함수의 Lock을 호출할경우
/// Funtion() <- 해당 함수 안에서 m.lock()을 또 잡을 경우가 생김
/// 
/// 2. 상호배타적 특성이 생길 수 있다.
/// 모든 애들이 리드만 한다면, 변경되는 데이터가 아니라면 굳이 락을 걸 필요가 없다.
/// 하지만 락이 변경되는 상황이 생길 수 있다. 그럴 경우 락을 쓴는데, 
/// 대부분은 변경된 사항보다는 읽기 상황이 주로 발생한다, 단순히 일기를 하는데 락이 필요할까?
/// 수정이 있을경우에만 상호배타적 락을 활성화  시킨다.
/// 수정이 없을경우에는 공유적으로 데이터 접근이 가능하다. <- 쓸데없는 경합을 막기 때문에 성능상 이점이 있다.
/// 
/// 3. 커스터 마이징 하는 이유.
/// 만드는 이유, 우리가 원하는 대로 만들 수 있다. <- 원하는대로 최적화 가능
/// 우리가 원하는 정책 선택가능
/// 해당 락을 추적해서 데드락 상황을 방지하는 코드 임포트 가능
///
/// 프로젝트마다 기존에 있는 RWLock을 쓸지 아니면 만들지 정해짐. 프로젝트마다 다름
/// 
/// 
/// WriteLock 조건
/// 1. 누구도 Lock을 점유한 상태가 아니여야 한다.
/// 2. 이미 같은 쓰레드가 WriteLock에 점유 하는 중이라면 WriteCount만 증가시켜준다.
/// 3. WriteCount는 같은 쓰레드가 점유할 경우에만 올라가기 때문에 Atomic을 보장할 필요가 없다.
/// 
/// LockGuard를 만들면 자동 Lock Unlock이 이루어지므로, 꼬이는 문제를 해결 할 수 있다.
