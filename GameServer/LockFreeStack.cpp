#include "pch.h"
#include "LockFreeStack.h"


void InitializeHeader(SListHeader* header) {
	header->alignment = 0;
	header->region = 0;
}


void PushEntryList(SListHeader* header, SListEntry* entry) {
	SListHeader expected = {};	
	SListHeader desired = {};	
	
	//16byte 정렬은 하위 4bit는 0000이다. 
	desired.HeaderX64.next = (((uint64)entry) >> 4); //60bit에 저장, 16bit로 정렬되었기 때문에 쉬프트연산으로 4비트 땡김.
														//-> 땡기는 이유 앞 4bit 공간을 다른 목적으로 사용 가능
	while (true) {
		expected = *header;

		//이 사이에 변경 가능하다. <- 다른쓰레드가
		entry->next = (SListEntry*)(((uint64)expected.HeaderX64.next) << 4); //header 복원
		desired.HeaderX64.depth = expected.HeaderX64.depth + 1;		 
		desired.HeaderX64.sequence = expected.HeaderX64.sequence + 1; 

		if (::InterlockedCompareExchange128((int64*)header, desired.region, desired.alignment, (int64*)&expected) == 1)
			break;
	}
}

SListEntry* PopEntryList(SListHeader* header) {
	SListHeader expected = {};	
	SListHeader desired = {};	
	SListEntry* entry = nullptr;
	
	while (true) {
		expected = *header;

		entry = (SListEntry*)(((uint64)expected.HeaderX64.next) << 4);
		if (entry == nullptr)
			break;

		//Use-After-Free 문제는 여전히 존재.
		desired.HeaderX64.next = ((uint64)entry->next) >> 4;
		desired.HeaderX64.depth = expected.HeaderX64.depth - 1;
		desired.HeaderX64.sequence = expected.HeaderX64.sequence + 1;

		if (::InterlockedCompareExchange128((int64*)header, desired.region, desired.alignment, (int64*)&expected) == 1)
			break;
	}
	return entry;
}

//해당 코드 ABA 문제점은 해결되었다.
//하지만 Use-After-Free문제는 여전히 존재합니다.
//LockFreeStack은 만들어 쓰는것보다 MS에서 제공해주는것 사용하기.
//MS에서도 16byte 정렬을 맞추라고 하는데, 이 구현 방법처럼 작동하기 때문이다.
//하위 4bit는 0000으로 확신이 가능하기 때문에.
