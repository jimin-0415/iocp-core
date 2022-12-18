#pragma once

//메모리에 1byte로 정렬 , 중간 구멍이 생기지 않는다.
#pragma pack(1)
struct BuffData 
{
	uint64 buffId;
	float remianTime;
};

struct SCT_TEST {
	uint64 id;
	uint32 hp;
	uint16 attack;

	////가변 데이터 
	////1. 문자열 (ex : name)
	////2. 그냥 바이트 배열 (ex : 길드 이미지)
	////3. 그냥 일반적인 정보 리스트.
	vector<BuffData> buffers;
	wstring name;
};

//iterator 지원
template<typename T, typename C>
class PacketIterator
{
public:
	PacketIterator(C& container, uint16 index) : _container(container), _index(index) { }

	bool operator!=(const PacketIterator& other) const { return _index != other._index; }
	const T& operator*() const { return _container[_index]; }
	T& operator*() { return _container[_index]; }
	T* operator->() { return &_container[_index]; }
	PacketIterator& operator++() { _index++; return *this; }
	PacketIterator	operator++(int32) { PacketIterator ret = *this; ++_index; return ret; }

private:
	C& _container;
	uint16 _index;
};

template<typename T>
class PacketList
{
public:
	PacketList() : _data(nullptr), _count(0) { }
	PacketList(T* data, uint16 count) : _data(data), _count(count) { }

	T& operator[] (uint16 index) {
		ASSERT_CRASH(index < _count);
		return _data[index];
	}

	uint16 Count() { return _count; }
	
	//range-for 지원
	PacketIterator<T, PacketList<T>> begin() { return PacketIterator<T, PacketList<T>>(*this, 0);}
	PacketIterator<T, PacketList<T>> end() { return PacketIterator<T, PacketList<T>>(*this, _count); }
private:
	T* _data;
	uint16 _count;
};

//패킷 임시 구조체
//[ PKT_S_TEST 고정사이즈 ][ 가변데이터 BuffListItem BuffListItem BuffListItem ...  ][victim victim][victim victim][victim victim]
struct PKT_S_TEST 
{
	struct BuffsListItem 
	{
		uint64 buffId;
		float remianTime;

		uint16 victimOffset;
		uint16 victimCount;
		
		//Size도 증가시켜준다.
		bool Validate(BYTE* packetStart, uint16 packetSize, OUT uint32& size) {
			if (victimOffset + victimCount * sizeof(uint64) > packetSize)
				return false;

			size += victimCount * sizeof(uint64);
			return true;
		}
	};

	uint16 packetSize; //공용 헤더 를 패킷에 포함 시켜서 전체 크기를 확인한다.
	uint16 packetId;	//공용헤더
	uint64 id;
	uint32 hp;
	uint16 attack;
	uint16 buffsOffset; //가변데이터 스타트위치 //대신.. 안에 하나씪 들어갈 경우 offset, count 가 계속 들어간다.
	uint16 buffsCount;
	
	bool Validate() {
		uint32 size = 0;
		
		size += sizeof(PKT_S_TEST);	//PK_S_TEST의 사이즈 계산
		if (packetSize < size)
			return false;

		if (buffsOffset + buffsCount + sizeof(BuffsListItem) > packetSize) //Offset 2차 테스트
			return false;

		//buffers 가변 데이터 크기 추가
		size += buffsCount * sizeof(BuffsListItem); //가변 길이의 사이즈 계산
		
		BuffList buffList = GetBuffsList();
		for (int32 i = 0; i < buffList.Count(); i++) {
			if (buffList[i].Validate((BYTE*)this, packetSize, OUT size) == false)
				return false;
		}

		//최종 크기 비교
		if (size != packetSize)	
			return false;

		return true;
	}

	using BuffList = PacketList<PKT_S_TEST::BuffsListItem>;
	using BuffVictimList = PacketList<uint64>;

	BuffList GetBuffsList() 
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffsOffset;
		return BuffList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
	}

	BuffVictimList GetBuffVictimList(BuffsListItem* buffsItem) 
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffsItem->victimOffset;
		return BuffVictimList(reinterpret_cast<uint64*>(data), buffsItem->victimCount);
	}
};
#pragma pack()

//Client To Server Test
//C_T_S
enum
{
	C_T_S_TEST = 1,
};

/// <summary>
/// ClientPacketHandler
/// </summary>
class ClientPacketHandler
{
public:
	//모든 패킷을 관리한다.
	static void HandlePacket(BYTE* buffer, int32 len);

	static void Handle_CTS_TEST(BYTE* buffer, int32 len);
};

