#pragma once


/// <summary>
/// //SingleThread LockFreeStack
/// </summary>

//16byte로 정렬
DECLSPEC_ALIGN(16)
struct SListEntry 
{
	SListEntry* next;
};

DECLSPEC_ALIGN(16)
class Data // : public SListEntry
{

public:
	SListEntry _entry;
	int64 _rand = rand() % 1000;
};

DECLSPEC_ALIGN(16)
struct SListHeader 
{
	SListHeader() {
		alignment = 0;
		region = 0;
	}
	union 
	{
		struct
		{
			uint64 alignment;
			uint64 region;
		} DUMMYSTRUCTNAME;

		struct
		{
			uint64 depth : 16;		//data 가 들어간 개수에 따라 1씩 증가
			uint64 sequence : 48;	//
			uint64 reserved : 4;
			uint64 next : 60;
		} HeaderX64;
	};
};


void InitializeHeader(SListHeader* header);
void PushEntryList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntryList(SListHeader* header);


