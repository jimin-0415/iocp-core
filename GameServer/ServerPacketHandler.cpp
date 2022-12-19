#include "pch.h"
#include "ServerPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"


PacketHandlerFunc GPacketHandler[UINT16_MAX];
//작업 코스

bool Handle_Invalid(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	cout << "WARN : " << header->id << endl;
	return false;
}

bool Handle_S_Test(PacketSessionRef& session, Protocol::S_TEST& pkt)
{
	return false;
}
