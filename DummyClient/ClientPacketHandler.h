#pragma once

struct BuffData 
{
	uint64 buffId;
	float remianTime;
};

//패킷 임시 구조체
struct SCT_TEST {
	uint64 id;
	uint32 hp;
	uint16 attack;

	//가변 데이터 
	//1. 문자열 (ex : name)
	//2. 그냥 바이트 배열 (ex : 길드 이미지)
	//3. 그냥 일반적인 정보 리스트.
	vector<BuffData> buffers;
};

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

