#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "Protocol.pb.h"
void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);
	PacketHeader header;
	br >> header;

	//Protocol Id 에 따라서 별도 처리
	//Switch Case 도 좋지만
	switch (header.id) {
	case S_TEST:
		Handle_CTS_TEST(buffer, len);
		break;
	}
}

void ClientPacketHandler::Handle_CTS_TEST(BYTE* buffer, int32 len)
{
	Protocol::S_TEST pkt;
	
	ASSERT_CRASH(pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)));

	cout << pkt.id() << " " << pkt.hp() << " " << pkt.attack() << endl;

	cout << "BUFF SIZE : " << pkt.buffs_size() << endl;

	for (auto& buff : pkt.buffs()) {
		cout << "BUFFINFO : " << buff.buffid() << " " << buff.remaintime() << endl;
		cout << "VICTIMS : " << buff.victims_size() << endl;
		
		for (auto& vic : buff.victims()) {
			cout << vic  << " ";
		}
		cout << endl;
	}
}
