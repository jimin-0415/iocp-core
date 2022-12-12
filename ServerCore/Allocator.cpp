#include "pch.h"
#include "Allocator.h"
#include "Memory.h"
void* BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
	return ::free(ptr);
}

void* StompAllocator::Alloc(int32 size)
{
	// [        4kb         [allocate]]
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	const int64 dataOffset = pageCount * PAGE_SIZE - size;
	void* baseAddress =::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	//								1byte 로 치환 후 offset 이동
	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset);
}

void StompAllocator::Release(void* ptr)
{
	//[				[allocator]] <- 할당 초기 메모리로 떙긴 후 메모리 할당 해제
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address - (address % PAGE_SIZE); //[ /////// []] 해당 크기 잡음
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}

void* PoolAllocator::Alloc(int32 size)
{
	return GMemory->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	GMemory->Release(ptr);
}
