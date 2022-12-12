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

///
/// StompAllocator
/// 메모리 오염 버그를 찾기에 좋은 Allocator 이다.
/// 장점 : 이미 메모리 해제된 [오염된] 메모리를 찾기 좋다.
/// 단점 : 작은 메모리 할당에도 큰 Page가 할당된다. 4byte 만 사용해도 4kbyte 크기의 페이지 할당
///		그렇기 떄문에 메모리 오버플로우 문제는 잡지 못한다. 작은 메모리에도 계속적으로 큰 페이지 크기만큼 할단
///		나중에 메모리단편화 문제 발생
/// 

class StompAllocator 
{
	enum { PAGE_SIZE = 0x1000 };	//4kb
public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};
