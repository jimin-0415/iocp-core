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
	//size 를 어떻게 걸를것이냐? 딱히 방법이 없다. 그럼 보안은 어떻게 할건데?
	//어떻게 할것이냐 >> 에서 넣을떄 Read할떄 , 계쏙 true false를 반환해서 검증하느것처럼
	//데이터가 가능한건지 체크를 하면 문제는 우아한 방법은 아니다, 이렇게 데이터 밀어넣고 파싱하면 온라인 게임에서 주로 일어난다.
	//데이터 추출할떄마다.. 검증하는건 별루다.

	//직렬화의 여러 기법에 대해서.

	BufferReader br(buffer, len);

	//if (len < sizeof(PKT_S_TEST))
	//	return;

	PKT_S_TEST* pkt = reinterpret_cast<PKT_S_TEST*>(buffer); 
	//해당 주소를 그냥 꺼내 쓴다. 현재 4byte는 안전하지만 다음 영역은 정상적인 데이터인지 모른다.
	//복사보단 캐스팅으로 버퍼를 바로 사용

	if (pkt->Validate() == false)
		return;

	//가변 데이터도 바로 접근함
	//복사해서 꺼내쓸 필요 없어진다.
	PKT_S_TEST::BuffList buffs = pkt->GetBuffsList();

	cout << "BuffCount " << buffs.Count() << endl;
	
	//range for 문법
	for (auto buff : buffs) {
		cout << "BuffInfo : " << buff.buffId << "  RemainTime : " << buff.remianTime << endl;

		PKT_S_TEST::BuffVictimList list = pkt->GetBuffVictimList(&buff);

		cout << "VictimeCount : " << list.Count() << endl;
		for (auto victim : list) {
			cout << "victim : " << victim << endl;
		}
	}


}
