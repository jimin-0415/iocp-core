#pragma once

//Client To Server Test
//C_T_S
enum
{
	S_TEST = 1,
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

