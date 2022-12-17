#pragma once

struct BuffData
{
	uint64 buffId;
	float remianTime;
};

enum 
{
	C_T_S_TEST =1,
};

class ServerPacketHandler
{
public:
	//모든 패킷을 관리한다.
	static void HandlePacket(BYTE* buffer, int32 len);

	static SendBufferRef Make_CTS_TEST(uint64 id, uint32 hp, uint16 attack, vector<BuffData> buffDatas);

	static void Handle_CTS_TEST(BYTE* buffer, int32 len);
};

