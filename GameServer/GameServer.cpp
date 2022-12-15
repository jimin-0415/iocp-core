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

#include "Service.h"
#include "Session.h"


class GameSession : public Session
{
public:
    ~GameSession() {
        cout << "~GameSession" << endl;
    }

    virtual int32 OnRecv(BYTE* buffer, int32 len) override
    {
        cout << "OnRecv Len = " << len << endl;
        Send(buffer, len);
        return len;
    }

    virtual void OnSend(int32 len) override {
        cout << "OnSend Len " << len << endl;
    }
};

int main()
{
    ServerServiceRef service = MakeShared<ServerService>(
        NetAddress(L"127.0.0.1", 7777)
        ,MakeShared<IocpCore>()
        ,MakeShared<GameSession>,   //SessionManager 등 ..
        100);
    
    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 5; i++) {
        GThreadManager->Launch([=]() {
            while (true) {
                service->GetIocpCore()->Dispatch();
            }
            });
    }
    return 0;
}

