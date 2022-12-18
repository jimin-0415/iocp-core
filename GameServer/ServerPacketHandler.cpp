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

