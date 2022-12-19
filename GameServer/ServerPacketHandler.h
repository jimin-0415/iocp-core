#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)> ;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

//TODO 자동화
enum : uint16
{
	PKT_S_TEST = 1,
};

bool Handle_Invalid(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_Test(PacketSessionRef& session, Protocol::S_TEST& pkt);
class ServerPacketHandler
{
public:
	//TODO 자동화
	static void Init() {
		for (int32 i = 0; i < UINT16_MAX; i++) {
			GPacketHandler[i] = Handle_Invalid;	//핸들 초기화
		}

		GPacketHandler[PKT_S_TEST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {
			return HandlePacket<Protocol::S_TEST>(Handle_S_Test, session, buffer, len);
		};
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len) {
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef&session, BYTE* buffer, int32 len) {
		PacketType pkt;

		if (false == pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)))
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId) {
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);

		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);
		return sendBuffer;
	}

public:

	//TODO : 자동화
	static SendBufferRef MakeSendBuffer(Protocol::S_TEST& pkt) { return MakeSendBuffer(pkt, PKT_S_TEST); }
};


