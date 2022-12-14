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

#include "RefCounting.h"
#include "Memory.h"
#include "ObjectPool.h"

#include "TypeCast.h"
#include "SocketUtils.h"
#include "Listener.h"
void HandleError(const char* cuase) {
    int errCode = ::WSAGetLastError();
    cout << cuase << errCode << endl;
}

const int32 BUFSIZE = 1000;
struct Session {
    SOCKET socket;
    char recvBuffer[BUFSIZE] = {};
    int32 recvBytes = 0;
    int32 sendBytes = 0;
};

int main()
{
    Listener listener;
    listener.StartAccept(NetAddress(L"127.0.0.1", 7777));

    for (int32 i = 0; i < 5; i++) {
        GThreadManager->Launch([]() {
            while (true) {
                GIocpCore.Dispatch();
                }
            });
    }

    GThreadManager->Join();

    return 0;
}

