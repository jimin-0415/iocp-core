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

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

void HandleError(const char* cuase) {
    int errCode = ::WSAGetLastError();
    cout << cuase << errCode << endl;
}

int main()
{
    //Winsock 라이브러리 초기화 ws2_32 초기화
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        HandleError("Socket");
        return 0;
    }

    SOCKADDR_IN serverAddr; //ip4 일경우 이 구조체 사용
    ::memset(&serverAddr, 0, sizeof(serverAddr)); //메모리 초기화
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);   //모든 주소 연결 
    serverAddr.sin_port = ::htons(7777);    //http : 80 같은 번호는 예약이 되어 있다.

    if (::bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        HandleError("Bind");
        return 0;
    }

    //udp 는 accept , listen 이 없다.

    while (true) {
        SOCKADDR_IN clientAddr;
        ::memset(&clientAddr, 0, sizeof(clientAddr));
        int32 addrLen = sizeof(clientAddr);

        char recvBuffer[1000];

        //accept 와 recv 가 섞인 느낌
        int32 recvLen = ::recvfrom(serverSocket, recvBuffer, sizeof(recvBuffer), 0
            ,(SOCKADDR*)&clientAddr, &addrLen);

        if (recvLen <= 0) {
            HandleError("RecvFrom");
            return 0;
        }

        cout << "Recv Data ! Data : " << recvBuffer << endl;
        cout << "Recv Data ! Len : " << recvLen << endl;

        int32 errorCode =::sendto(serverSocket, recvBuffer, recvLen, 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));

        if (errorCode == SOCKET_ERROR) {
            HandleError("SendTo");
            return 0;
        }

        cout << "Send Data! Len = " << recvLen << endl;
    }

    ::WSACleanup();

	return 0;
}

