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
    SOCKET socket = SocketUtils::CreateSocket();

    SocketUtils::BindAnyAddress(socket, 7777);
    SocketUtils::Listen(socket);

    SOCKET clientSocket = ::accept(socket, nullptr, nullptr);

    cout << "Client Connected!" << endl;

    while (true) {

    }

    GThreadManager->Join();
    return 0;
}

