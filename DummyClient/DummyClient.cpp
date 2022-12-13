#include "pch.h"

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

    //af : address Familiy 주소 체계 
    //type : Tcp, Udp
    //protocol : 0
    // return : descriptor <- 정수 받은 각 socket 의 식별 번호
    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        int errCode = ::WSAGetLastError();
        cout << "Socket ErrorCode :" << errCode << endl;
        return 0;
    }

    SOCKADDR_IN serverAddr; //ip4 일경우 이 구조체 사용
    ::memset(&serverAddr, 0, sizeof(serverAddr)); //메모리 초기화
    serverAddr.sin_family = AF_INET;
    //serverAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1") //너무 옛날 버전
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = ::htons(7777);    //http : 80 같은 번호는 예약이 되어 있다.

    //host to network short
    //Little-Endian vs Big-Endian
    //ex) 0x12345678    4바이트 정수 저장
    // low [0x78][0x56][0x34][0x12] high <   little , cpu에 따라서 다르다, 요즘은 모두 little 엔디안 방식 [client] 
    // <- 로컬 컴퓨터 환경은 little 방식이다
     // low [0x12][0x34][0x56][0x78] high <   Big -> 받는 쪽은 반대로 받아야 한다. bigEndian 방식으로. [server]
    
    if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        int errCode = ::WSAGetLastError();
        cout << "Connet ErrorCode :" << errCode << endl;
        return 0;
    }

    //Connect Success
    cout << "Connected To Server!" << endl;

    while (true) {

        char sendBuffer[100] = "Hello World!";
        
        for (int i = 0; i < 10; ++i) {
            int32 resultCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
            if (resultCode == SOCKET_ERROR) {
                int errCode = ::WSAGetLastError();
                cout << "send ErrorCode :" << errCode << endl;
                return 0;
            }

            cout << "Send Data ! size : " << sizeof(sendBuffer) << endl;
        }
        
        this_thread::sleep_for(1s);
    }

    //소켓 리소스 반환
    ::closesocket(clientSocket);

    //윈속 종료
    ::WSACleanup();
}

