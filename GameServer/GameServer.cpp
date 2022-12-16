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

    GThreadManager->Join();
    return 0;
}

