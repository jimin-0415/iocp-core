#pragma once

enum
{
	SLIST_ALIGNMENT = 16
};

//[Memory Header][Data]
DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
	MemoryHeader(int32 size)
		:allocSize(size)
	{
	}

	static void* AttachHeader(MemoryHeader* header, int32 size) {
		new(header)MemoryHeader(size);
		//return [Data] Start Point - ++해서 memoryHeader크기만큼 건너뛰기.
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetachHeader(void* ptr) {
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}

	int32 allocSize;
};

/// <summary>
/// MemoryPool class
/// </summary>

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	void Push(MemoryHeader* ptr);
	MemoryHeader* Pop();

private:
	SLIST_HEADER	_header; 
	int32			_allocSize = 0;
	atomic<int32>	_allocCount = 0;
};

