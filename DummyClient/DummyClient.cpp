#include "pch.h"

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
    this_thread::sleep_for(1s);
    //Winsock 라이브러리 초기화 ws2_32 초기화
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        HandleError("Socket");
        return 0;
    }

    //socket 의 io Mode 제거
    u_long on = 1;
    if (::ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
        return 0;

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = ::htons(7777);

    while (true) {
        if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                continue;

            //이미 연결된 상태일 경우 
            if (::WSAGetLastError() == WSAEISCONN)
                break;

            //Error
            break;
        }
    }

    cout << "Connected to Server !" << endl;

    char sendBuffer[100] = "Hello World";

    //Send
    while (true) {
        if (::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0) == SOCKET_ERROR) {
            //블록 여부 확인
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                continue;

            //Error
            break;
        }

        cout << "Send Data ! Len = " << sizeof(sendBuffer) << endl;

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

            break;
        }
        this_thread::sleep_for(1s);
    }

    //소켓 리소스 반환
    ::closesocket(clientSocket);

    //윈속 종료
    ::WSACleanup();
}

