#pragma once
/// <summary>
/// Allocator
/// 할당 정책을 해당 클래스에 정의합니다.
/// </summary>


/// <summary>
/// BaseAllocator
/// </summary>
class BaseAllocator
{
public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};

