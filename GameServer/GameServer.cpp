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

int main()
{
    //Winsock 라이브러리 초기화 ws2_32 초기화
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        int errCode = ::WSAGetLastError();
        cout << "Socket ErrorCode :" << errCode << endl;
        return 0;
    }

    SOCKADDR_IN serverAddr; //ip4 일경우 이 구조체 사용
    ::memset(&serverAddr, 0, sizeof(serverAddr)); //메모리 초기화
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);   //모든 주소 연결 
    serverAddr.sin_port = ::htons(7777);    //http : 80 같은 번호는 예약이 되어 있다.

    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        int errCode = ::WSAGetLastError();
        cout << "Bind ErrorCode :" << errCode << endl;
        return 0;
    }

    //일단 대기열 10개
    if (::listen(listenSocket, 10) == SOCKET_ERROR) {
        int errCode = ::WSAGetLastError();
        cout << "Listen ErrorCode :" << errCode << endl;
        return 0;
    }

    while (true) {
        SOCKADDR_IN clientAddr;
        ::memset(&clientAddr, 0, sizeof(clientAddr)); //메모리 초기화
        int32 addresslen = sizeof(clientAddr);

        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addresslen);
        if (clientSocket == INVALID_SOCKET) {
            int errCode = ::WSAGetLastError();
            cout << "Socket Accept ErrorCode :" << errCode << endl;
            return 0;
        }

        char ipAddress[16];
        ::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
        cout << "Client Connected IP = " << ipAddress << endl;

        while (true) {
            //몇바이트를 보낼지 예측 불가
            char recvBuffer[1000];
            int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (recvLen <= 0) {
                int errCode = ::WSAGetLastError();
                cout << "recv Accept ErrorCode :" << errCode << endl;
                return 0;
            }

            cout << "Recv Data ! Data = " << recvBuffer << endl;
            cout << "Recv Data Len = " << recvLen << endl;

            char sendBuffer[100] = "Hello World!";
            int32 resultCode = ::send(clientSocket, recvBuffer, recvLen, 0);
            if (resultCode == SOCKET_ERROR) {
                int errCode = ::WSAGetLastError();
                cout << "send ErrorCode :" << errCode << endl;
                return 0;
            }

            cout << "Send Data ! size : " << sizeof(sendBuffer) << endl;

        }
    }

    ::WSACleanup();
	return 0;
}

