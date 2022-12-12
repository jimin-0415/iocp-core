#include "pch.h"
#include "Allocator.h"

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
	//4 -> 4096	
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	return ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void StompAllocator::Release(void* ptr)
{
	::VirtualFree(ptr, 0, MEM_RELEASE);
}
