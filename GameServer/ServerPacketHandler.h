#pragma once
#include "BufferReader.h"
#include "BufferWriter.h"

enum
{
	C_T_S_TEST = 1,
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
	PacketIterator<T, PacketList<T>> begin() { return PacketIterator<T, PacketList<T>>(*this, 0); }
	PacketIterator<T, PacketList<T>> end() { return PacketIterator<T, PacketList<T>>(*this, _count); }
private:
	T* _data;
	uint16 _count;
};
#pragma pack(1)
//[ PKT_S_TEST 고정사이즈 ][ 가변데이터 BuffListItem BuffListItem BuffListItem ...  ]
struct PKT_S_TEST {
	struct BuffsListItem {
		uint64 buffId;
		float remianTime;

		//Victim List
		uint16 victimsOffset;
		uint16 victimsCount;
	};

	uint16 packetSize; //공용 헤더 를 패킷에 포함 시켜서 전체 크기를 확인한다.
	uint16 packetId;	//공용헤더
	uint64 id;
	uint32 hp;
	uint16 attack;
	uint16 buffsOffset; //가변데이터 스타트위치 //대신.. 안에 하나씪 들어갈 경우 offset, count 가 계속 들어간다.
	uint16 buffsCount;

	//validate 는 받는 쪽에서 하는것이기 때문에 보내는 쪽에서는 필요 x 
	//bool Validate() {
	//	uint32 size = 0;

	//	size += sizeof(PKT_S_TEST);	//PK_S_TEST의 사이즈 계산
	//	if (packetSize < size)
	//		return false;

	//	size += buffsCount * sizeof(BuffsListItem); //가변 길이의 사이즈 계산
	//	if (size != packetSize)	//PaketSize 테스트
	//		return false;

	//	if (buffsOffset + buffsCount + sizeof(BuffsListItem) > packetSize) //Offset 2차 테스트
	//		return false;
	//	return true;
	//}

	/*using BuffList = PacketList<PKT_S_TEST::BuffsListItem>;

	BuffList GetBuffsList()
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffsOffset;
		return BuffList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
	}*/
};

//정보를 나타내는 클래스
//[PKT_S_TEST][BuffsListItem BuffsListItem BuffsListItem]
//만약 vector, List 처럼 하나씩 넣게 되면 문제가 된다: 이유<> 동일한 오브젝트가 아닌, 다른 오브젝트가 중간에 넣어질경우 복잡성 증가
//개수를 먼저 딱 지정해야함. 그렇게 메모리 할당하고 채운다.
class PKT_S_TEST_WRITE 
{
public:
	using BuffListItem = PKT_S_TEST::BuffsListItem;
	using BuffList = PacketList<PKT_S_TEST::BuffsListItem>;
	using BuffVictimsList = PacketList<uint64>;
	//고정
	PKT_S_TEST_WRITE(uint64 id, uint32 hp, uint16 attack)
	{
		_sendBuffer = GSendBufferManager->Open(4096);
		_bw = BufferWriter(_sendBuffer->Buffer(), _sendBuffer->AllocSize());

		_pkt = _bw.Reserve<PKT_S_TEST>();
		_pkt->packetSize = 0; //To Fill
		_pkt->packetId = C_T_S_TEST;
		_pkt->id = id;
		_pkt->hp = hp;
		_pkt->attack = attack;
		_pkt->buffsOffset = 0; //TO Fill
		_pkt->buffsCount = 0; //TO Fill
	}

	//[PKT_S_TEST] [예약 예약 예약]
	BuffList ReserveBuffList(uint16 buffCount) 
	{
		//victime이 오면서 4byte 추가
		BuffListItem* firstBuffListItem = _bw.Reserve<BuffListItem>(buffCount);
		_pkt->buffsOffset = (uint64)firstBuffListItem - (uint64)_pkt;
		_pkt->buffsCount = buffCount;
		
		return BuffList(firstBuffListItem, buffCount);
	}

	BuffVictimsList ReserveBuffsVictimsList(BuffListItem* buffsItem, uint16 victimsCount) 
	{
		uint64* fritstVictimsListItem = _bw.Reserve<uint64>(victimsCount);
		buffsItem->victimsOffset = (uint64)fritstVictimsListItem - (uint64)_pkt;
		buffsItem->victimsCount = victimsCount;

		return BuffVictimsList(fritstVictimsListItem, victimsCount);
	}

	SendBufferRef CloseAndReturn() 
	{
		_pkt->packetSize = _bw.WriteSize();
		
		_sendBuffer->Close(_bw.WriteSize());
		return _sendBuffer;
	}

private:
	PKT_S_TEST* _pkt = nullptr;
	SendBufferRef _sendBuffer;
	BufferWriter _bw;
};

#pragma pack()

class ServerPacketHandler
{
public:
	//모든 패킷을 관리한다.
	static void HandlePacket(BYTE* buffer, int32 len);
};


