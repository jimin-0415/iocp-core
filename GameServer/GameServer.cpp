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

    //커널의 RECV, SEND 버퍼 크기도 수정이 가능하다.
    //옵션을 해석하고 처리할 주체? [level]
    //소켓 코드 -> SOL_SOKET
    // IPv4 -> IPPROTO_IP
    // TCP 프로토콜 -> IPPROTO_TCP

    // SO_KEEPALIVE //주기적으로 연결 상태 여부 확인 (TCP Only)
    // 상대방이 소리 소문 없이 연결을 끊는경우가 있기 때문에..
    // 주기 적으로 TCP 프로토콜 연결 상태 확인 -> 끊어진 연결 감지.

    bool enable = true;
    ::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));

    // SO_LINGER = 지연하다.
    // 송신 버퍼에 있는 데이터를 보낼것인가? 날릴것인가?
    // Send -> closesocket 을 하면 Send 에서 보낸 버퍼에 남은 데이터는 어떻게 할것이냐
    
    // onoff =0 이면 closesocket()이 바로 리턴, 아니면 linger 초만큼 대기 (default 0)
    LINGER linger;
    linger.l_onoff = 1; //활성화
    linger.l_linger = 5; //s 단위 5초
    ::setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

    // Half - Close
    // SD_SEND : send만 막는다.
    // SD_RECEIVE : recv 막는다.
    // SD_BOTH : 둘다 막는다.
    // shutdown  전에 해당 소켓설정을 한다.
    ::shutdown(serverSocket, SD_SEND);  

    //송 수신 버퍼 크기를 4gb 그렇게 크게 못한다.
    // 문서를 보고 사용할 애들값을 세팅해 준다.
    // SO_SNDBUF = 송신버퍼 크기
    // SO_RCVBUF = 수신버퍼 크기
    int32 sendBufferSize;
    int32 optionLen = sizeof(sendBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &optionLen);
    cout << "송신 버퍼 크기 : " << sendBufferSize << endl;

    int32 recvBufferSize;
    optionLen = sizeof(recvBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&recvBufferSize, &optionLen);
    cout << "송신 버퍼 크기 : " << recvBufferSize << endl;

    // SO_REUSEADDR
    // IP 주소 및 Port를 재사용 한다.
    // 다른 프로그램 사용 혹은 서버를 강제종료하고 다시 켰을 경우 해당 찌끄레기가 남아서 해당 IP Port 바인딩 안되는 상황이 생김.
    // 이때 해당 옵션을 통해서 IP Port를 재사용한다.
    {
        //개발 단계에서 편하라고 사용함.
        bool enable = true;
        ::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));
    }

    // IPPROTO_TCP
    // TCP_NODELAY = Nagle 네이글 알고리즘 작동 여부
    // 데이터가 충분히 크면 보내고, 그렇지 않으면 데이터가 충분히 쌓일때까지 대기 !
    // 장점 : 작은 패킷이 불필요하게 많이 생성되는 일을 방지한다.
    // 단점 : 반응 시간을 손해.
    // 게임쪽에서는 거의 사용 안함. <- 직접 관리. 반응성 문제.
    {
        bool enable = true; // nagle 작동 안함.
        ::setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
    }

    ::WSACleanup();
	return 0;
}

