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
    //Winsock 라이브러리 초기화 ws2_32 초기화
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    //af : address Familiy 주소 체계 
    //type : Tcp, Udp
    //protocol : 0
    // return : descriptor <- 정수 받은 각 socket 의 식별 번호
    SOCKET clientSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        HandleError("Socket ErrorCode");
        return 0;
    }

    SOCKADDR_IN serverAddr; 
    ::memset(&serverAddr, 0, sizeof(serverAddr)); 
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = ::htons(7777);   

    //-Connected Udp -> 즐겨찾기 처럼 보낼 대상을 등록한다.
    ::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    //---------------------------

    while (true) {
        char sendBuffer[100] = "Hello World!";
        
        //--Unconnected Udp 
        //나의 ip주소, 포트번호가 설정됨.  
        /*int32 resultCode = ::sendto(clientSocket, sendBuffer, sizeof(sendBuffer), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
        if (resultCode == SOCKET_ERROR) {
            HandleError("send ErrorCode :");
            return 0;
        }*/
        //---

        //-Connected Udp -> 보낼 때는 sedn로 보낸다.
        int32 resultCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
        if (resultCode == SOCKET_ERROR) {
            HandleError("send ErrorCode :");
            return 0;
        }
        //---


        cout << "Send Data ! size : " << sizeof(sendBuffer) << endl;
        
        SOCKADDR_IN recvAddr;
        ::memset(&recvAddr, 0, sizeof(recvAddr));
        int32 addrLen = sizeof(recvAddr);

        char recvBuffer[1000];

        //--Unconnected Udp 
        /*int32 recvLen = ::recvfrom(clientSocket, recvBuffer, sizeof(recvBuffer), 0
            , (SOCKADDR*)&recvAddr, &addrLen);*/
        //-----

        //--Unconnected Udp 
        int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0
                    );
        //-----


        if (recvLen <= 0) {
            HandleError("RecvFrom");
            return 0;
        }

        cout << "Recv Data ! Data : " << recvBuffer << endl;
        cout << "Recv Data ! Len : " << recvLen << endl;

        this_thread::sleep_for(1s);
    }

    //소켓 리소스 반환
    ::closesocket(clientSocket);

    //윈속 종료
    ::WSACleanup();
}

