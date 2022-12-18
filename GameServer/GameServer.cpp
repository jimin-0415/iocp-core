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
        vector<BuffData> buffDatas{
            BuffData { 100, 1.5f },
            BuffData { 200, 1234.f },
            BuffData { 300, 99.f },
        };
        uint64 id = 1000;
        uint32 hp = 234;
        uint16 attack = 23;
        
        SendBufferRef sendBuffer = ServerPacketHandler::Make_CTS_TEST(id, hp, attack, buffDatas, L"안녕하세요");
            
        GSessionManager.BroadCast(sendBuffer);

        this_thread::sleep_for(200ms);
    }
    GThreadManager->Join();
    return 0;
}

