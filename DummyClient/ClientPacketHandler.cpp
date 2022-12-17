#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"

void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);
	PacketHeader header;
	br >> header;

	//Protocol Id 에 따라서 별도 처리
	//Switch Case 도 좋지만
	switch (header.id) {
	case C_T_S_TEST:
		Handle_CTS_TEST(buffer, len);
		break;
	}
}

void ClientPacketHandler::Handle_CTS_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);
	PacketHeader header;
	br >> header;

	uint64 id;
	uint32 hp;
	uint16 attack;
	br >> id >> hp >> attack;

	cout << "ID : " << id << " HP : " << hp << " Attack :" << attack << endl;

	//가변 길이기 때문에, 클라이언트는 절대 믿으면 안된다.
	vector<BuffData> buffDatas;
	uint16 buffCount;
	br >> buffCount;

	buffDatas.resize(buffCount);
	for (int32 i = 0; i < buffCount; i++) {
		br >> buffDatas[i].buffId >> buffDatas[i].remianTime;
	}

	cout << "BuffCount " << buffCount << endl;
	for (int32 i = 0; i < buffCount; i++) {
		cout << "BuffInfo : " << buffDatas[i].buffId << "  RemainTime : " << buffDatas[i].remianTime << endl;
	}
	//char recvBuffer[4096];
	////현재는 길이를 이렇게 보내지만 가변길이의 경우 가변길이의 Packet Size를 추가로 보내주면 된다. 일단은 임시로 
	////Protoocl 규약에 맞춰서 순서대로 패킷 정보를 넣어준다.
	//br.Read(recvBuffer, header.size - sizeof(PacketHeader) - sizeof(uint64) - sizeof(uint32) - sizeof(uint16));
	//cout << recvBuffer << endl;
}
