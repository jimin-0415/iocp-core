#include "pch.h"
#include "MemoryPool.h"

MemoryPool::MemoryPool(int32 allocSize)
	:_allocSize(allocSize)
{
	
}

MemoryPool::~MemoryPool()
{
}

void MemoryPool::Push(MemoryHeader* ptr)
{
	WRITE_LOCK;
	//일단 allocSize가 0일 경우 사용하지 않는다.
	ptr->allocSize = 0;
	_queue.push(ptr);

	_allocCount.fetch_sub(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* header = nullptr;

	{
		WRITE_LOCK;
		//pool에서 꺼내오기
		if (_queue.empty() == false) {
			header = _queue.front();
			_queue.pop();
		}
	}

	//Pool에 없을 경우 새로 할당
	if (header == nullptr) {
		header = reinterpret_cast<MemoryHeader*>(::malloc(_allocSize));
	}
	else{
		ASSERT_CRASH(header->allocSize == 0);
	}

	_allocCount.fetch_add(1);

	return header;
}
