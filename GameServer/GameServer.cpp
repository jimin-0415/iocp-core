#include "pch.h"
#include <windows.h>
#include <future>
#include <mutex>
#include <thread> 
#include <atomic>
#include <chrono>
#include "ThreadManager.h"
#include "PlayerManager.h"
#include "AccountManager.h"
#include "CoreGlobal.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"

int main()
{
    ServerServiceRef service = MakeShared<ServerService>(
        NetAddress(L"127.0.0.1", 7777)
        ,MakeShared<IocpCore>()
        ,MakeShared<GameSession>,   //SessionManager 등 ..
        5);
    
    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 1; i++) {
        GThreadManager->Launch([=]() {
            while (true) {
                service->GetIocpCore()->Dispatch();
            }
            });
    }

    char SendData[1000] = "Hello World!!!!!";

    while (true) {
        SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
        
        BYTE* buffer = sendBuffer->Buffer();
        
        //Size Id 를 통해서 엉뚱한 데이터 파싱 가능해짐. 데이터 조작 가능함.
        ((PacketHeader*)buffer)->size = (sizeof(SendData) + sizeof(PacketHeader));
        ((PacketHeader*)buffer)->id = 1; //Hello World CMD;

        ::memcpy(&buffer[4], SendData, sizeof(SendData));
        sendBuffer->Close(sizeof(SendData) + sizeof(PacketHeader));
        
        GSessionManager.BroadCast(sendBuffer);

        this_thread::sleep_for(300ms);
    }
    GThreadManager->Join();
    return 0;
}

