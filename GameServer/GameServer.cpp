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
#include "BufferWriter.h"
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

    while (true) {
        SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

        BufferWriter bw(sendBuffer->Buffer(), 4096);
        PacketHeader* header = bw.Reserve<PacketHeader>(); //PacketHeader 예약
        
        uint64 id = 1000;
        uint32 hp = 234;
        uint16 attack = 23;

        //성능향상을 위해서 굳이 길이까지 체크 안함 .
        bw << uint64(1000) << uint32(23) << uint16(444);
       
        bw.Write(SendData, sizeof(SendData));

        header->size = bw.WriteSize();
        header->id = 1; //packet Id
        
        sendBuffer->Close(bw.WriteSize());
        
        GSessionManager.BroadCast(sendBuffer);

        this_thread::sleep_for(200ms);
    }
    GThreadManager->Join();
    return 0;
}

