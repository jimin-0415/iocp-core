#pragma once

#include "Allocator.h"

class MemoryPool;

template<typename Type, typename... Args>
Type* xnew(Args&&... args) {
	Type* memory = static_cast<Type*>(XAlloc(sizeof(Type)));

	//placement new
	new(memory)Type(forward<Args>(args)...); //xvalue, rvalue 일경우 foward 를 통해서 넘겨준다.
	return memory;
}

template<typename Type>
void xdelete(Type* obj) {
	obj->~Type();	//객체를 대상으로 소멸자를 호출한다.
	XRelease(obj);	//메모리를 반납한다.
}

class Memory
{
	//~1024 => 32byte
	//~2048 => 128byte
	//~4096 => 256byte
	//4096 ~ => 기본 힙 할당기로 메모리 할당 =>4kb라면 꾀 큰 데이터이기 때문에 풀링할 필요없다.
	enum 
	{
		POOL_COUNT = (1024 / 32) + (2048 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096, 
	};

public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void Release(void* ptr);
private:
	vector<MemoryPool*> _pools;					//기본 vector 생성, 해당 vector는 직접 만든 메모리로 관리하면 안됨.
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1]; //Memory Find Helper Table //메모리를 빠르게 찾도록 한다.
};