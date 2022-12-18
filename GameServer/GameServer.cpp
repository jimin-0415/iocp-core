#include "pch.h"
#include <windows.h>
#include <future>
#include <mutex>
#include <thread> 
#include <atomic>
#include <chrono>
#include "ThreadManager.h"
#include "CoreGlobal.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ServerPacketHandler.h"
#include "tchar.h"
int main()
{
    ServerServiceRef service = MakeShared<ServerService>(
        NetAddress(L"127.0.0.1", 7777)
        ,MakeShared<IocpCore>()
        ,MakeShared<GameSession>,   //SessionManager 등 ..
        8);
    
    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 8; i++) {
        GThreadManager->Launch([=]() {
            while (true) {
                service->GetIocpCore()->Dispatch();
            }
            });
    }

    char SendData[1000] = "Hello World!!!!!"; //CP949 (한글2바이트) + 로마 1바이트
    char SendData2[1000] = u8"Hello World!!!!!"; //UTF-8 = unicode 한글3바이트 + 로마 1바이트
    WCHAR SendData3[1000] = L"Hello World!!!!!"; //UTF-16 = Unicode (한글/로마 2바이트) -> 모두 2바이트라 좋다
    TCHAR SendData4[1000] = _T("Hello World!!!!!"); //문자집합 속성에 따라서 달라짐, 기본 WCHAR

    //공통 부분은 함수로 관리하자.
    while (true) {

        //[PKT_S_TEST]
        PKT_S_TEST_WRITE pktWriter(1001, 100, 10);

        //[PKT_S_TEST][BuffListItem BuffListItem BuffListItem]
        PKT_S_TEST_WRITE::BuffList buffList = pktWriter.ReserveBuffList(3);
        buffList[0] = { 100, 1.5f };
        buffList[1] = { 200, 1234.f };
        buffList[2] = { 300, 24.f };
       
        PKT_S_TEST_WRITE::BuffVictimsList pktvictimList0 = pktWriter.ReserveBuffsVictimsList(&buffList[0], 3);
        {
            pktvictimList0[0] = 1000;
            pktvictimList0[1] = 1001;
            pktvictimList0[2] = 1002;
        }

        PKT_S_TEST_WRITE::BuffVictimsList pktvictimList1 = pktWriter.ReserveBuffsVictimsList(&buffList[1], 1);
        {
            pktvictimList1[0] = 2000;
        }

        PKT_S_TEST_WRITE::BuffVictimsList pktvictimList2 = pktWriter.ReserveBuffsVictimsList(&buffList[2], 2);
        {
            pktvictimList2[0] = 3000;
            pktvictimList2[1] = 3001;
        }

        SendBufferRef sendBuffer = pktWriter.CloseAndReturn();

        GSessionManager.BroadCast(sendBuffer);

        this_thread::sleep_for(200ms);
    }
    GThreadManager->Join();

    //가변이기 때문에, 해당 메모리 reserve 를 해야한다.
    //코드관리가 용이하지 않다.
    //데이터를 바로 밀어넣기때문에 장점은 있지만, 사용에 불편하다.

    //구글에 만든 Flat 버프 도 이런식으로 직접 버퍼에 데이터 밀어넣고 꺼낸디. 성능은 좋지만 사용에 지저분하다.
    //반면 proto버프는 실시간 리저브하지 않지 때문에 코드는 깔끔하지만, 복사 비용이 발생한다.

    return 0;
}

