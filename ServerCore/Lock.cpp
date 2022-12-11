#include "pch.h"
#include "Lock.h"
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	//동일 쓰레드일 경우 WriteLock성공
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK);
	if (LThreadId == lockThreadId) {
		_writeCount++;
		return;
	}

	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);	//WirteLock Masking
	const uint64 beginTick = ::GetTickCount64();	//ThreadLock Start Time
	while (true) {
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++) {
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired)) { //empty일 경우 WriteLock 획득
				_writeCount++;
				return;
			}
		}

		if (::GetTickCount64()- beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("Write Lock Time Out");

		this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	//ReadRock전에는 WriteUnlock은 불가능하다.
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
		CRASH("Invalid Unlock Order");

	const uint32 lockCount = --_writeCount;
	if (lockCount == 0)
		_lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	//동일한 쓰레드가 소유하고 있다면 ReadLockFlag 1증가
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId) {
		_lockFlag.fetch_add(1);
		return;
	}


	//아무도 소유하고 있지 않을 경우 [WriteLock을 소유하고 있지 않을 경우] 경합해서 공유 카운트 올린다.
	const uint64 beginTick = ::GetTickCount64();	//ThreadLock Start Time
	while (true) {
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++) {
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);	//0000 0000 , 0000 00011
			
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("Read Lock Time Out");
		
		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("Multiple Unlock");
}
