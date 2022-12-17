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

    char SendData[1000] = "Hello World!!!!!";

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
        
        SendBufferRef sendBuffer = ServerPacketHandler::Make_CTS_TEST(id, hp, attack, buffDatas);
            
        GSessionManager.BroadCast(sendBuffer);

        this_thread::sleep_for(200ms);
    }
    GThreadManager->Join();
    return 0;
}

