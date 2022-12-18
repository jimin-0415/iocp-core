#include "pch.h"
#include "ServerPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"

void ServerPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);
	
	PacketHeader header;
	br.Peek(&header);

	switch (header.id) {
	defualt:
		break;
	}
}

SendBufferRef ServerPacketHandler::Make_CTS_TEST(uint64 id, uint32 hp, uint16 attack, vector<BuffData> buffDatas, wstring name)
{
    SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

    BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
    
	PacketHeader* header = bw.Reserve<PacketHeader>(); //PacketHeader 예약
	
    bw << id << hp << attack;

	struct ListHeader
	{
		uint16 offset;
		uint16 count;
	};

	//가변데이터 
	ListHeader* buffersHeader = bw.Reserve<ListHeader>();
	buffersHeader->count = buffDatas.size();
	buffersHeader->offset = bw.WriteSize();	//시작위치

	for (BuffData& buff : buffDatas) {
		bw << buff.buffId << buff.remianTime;
	}

    header->size = bw.WriteSize();
    header->id = C_T_S_TEST; //packet Id

    sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}
