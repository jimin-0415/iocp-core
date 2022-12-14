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

//블로킹을 논 블로킹으로 바꾼다고 해서, 성능 이점이 없다.
//현재 코드를 보면 계속적으로 Connection, Send, Recv가 오는지 확인하기 위해서 while문으로 루프를 보고있다.
//불필요하게 체크를 하기 때문에 CPU를 계속 점유한 상태가 된다.
//1. 소켓은 옵션을 바꾸면 논블로킹으로 바뀌는구나.
//2. 논블로킹으로 바꾼다고 해서 모든 문제가 해결되는것은 아니다. - CPU 사이클 낭비를 줄여야 한다. 여러 소켓 모델로 해결

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

    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        return 0;

    //socket 의 io Mode 제거
    u_long on = 1;
    if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
        return 0;

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777);

    //기본과 동일
    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        return 0;

    //Listen 도 동일
    if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        return 0;

    cout << "Accept" << endl;

    SOCKADDR_IN clientAddr;
    int32 addrLen = sizeof(clientAddr);

    //Accept
    while (true) {
        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                continue;

            //Error
            break;
        }

        cout << "Client Connected !" << endl;

        //Recv
        while (true) {
            char recvBuffer[1000];
            int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (recvLen == SOCKET_ERROR) {
                if (::WSAGetLastError() == WSAEWOULDBLOCK)
                    continue;

                //Error
                break;
            }
            else if (recvLen == 0) {
                //연결 끊킴
                break;
            }

            cout << "Recv Data Len = " << recvLen << endl;

            //Send
            while (true) {
                if (::send(clientSocket, recvBuffer, recvLen, 0) == SOCKET_ERROR) {
                    if (::WSAGetLastError() == WSAEWOULDBLOCK)
                        continue;

                    //Error
                    break;
                }

                cout << "Send Data ! Len = " << recvLen << endl;
                break;
            }
        }

    }

    ::WSACleanup();
    return 0;
}

