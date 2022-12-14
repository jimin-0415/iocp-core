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

const int32 BUFSIZE = 1000;
struct Session {
    SOCKET socket;
    char recvBuffer[BUFSIZE] = {};
    int32 recvBytes = 0;
    int32 sendBytes = 0;
};

int main()
{
    //Winsock 라이브러리 초기화 ws2_32 초기화
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

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

    //SELECT 모델 = (select 함수가 핵심이 된다)
    // Socekt 함수 호출이 성공할 시점을 미리 알 수 있다. - blocking, Non-Blocking 모두 적용 가능.
    //문제 상황 > <- 수신 버퍼에 데이터가 없을때, read한다는 상황.
    //문제상황 2 > <- 송신버퍼가 꽉 찼을때 write한다거나 하는 상황.

    //- 블로킹 소켓 : 조건이 만족되지 않아서 블로킹 되는 상황 방지.
    //- 논블로킹 소켓 : 조건이 만족되지 않아서 불필요하게 반복체크하는 상황 예방.

    //socket set
    //1. 읽기[] 쓰기[] 예외 [] 관찰 대상을 등록한다. 모두 넣을 필요 없음
    //원하는 관찰 대상을 넣는다. 
    //예외 ? OutOfBand - Send() 마지막 인자 MSG_OOB로 특수 데이터를 보냄
    //받는 쪽에서도 recv OOB 세팅을 해야 읽을 수 있다.
    //2. select(readSet, writeSet, exceptSet); 을 넣어서 관찰 시작.
    //3. 적어도 하나의 소켓이 준비되면 리턴 <- 낙오자는 알아서 제거됨.
    // ex > 1,2,3 소켓을 대상으로 일기를 하는데.. 이것을 socekt set에 등록해서 체크를 한다.
    // 만약여기서 select를 호출하면 1.2.3을 관찰하다가 만약 2번에 데이터가 와서 수신버퍼가 왔다면 개수를 반환해 준다.
    // 그러면 읽기에는 2만 남고 나머지 1,3는 제거된다. -> 처리를 끝나면 다시 1,2,3을 읽기 select에 세팅한다.

    //함수 호출 을 체크하고 한다.
    ///장점 : 구현이 간단하고, 리소스 낭비를 줄일 수 있다.
    //담점 : 반복적으로 등록에 문제가 생김, FD_SETSIZE <- 64개까지밖에 등록을 못한다. fd_set 하나에...만약 640명이면 640개를 만들면된다.

    //fd_set read;
    // FD_ZERO : 비운다.
    //FD_ZERO(set);

    //FD_SET : 소켓 s를 넣는다.
    //FD_SET(s, &set);
    // 
    //FD_CLR : 소켓 s 를 제거
    //FD_CLR(s, &set);
    
    //FD_ISSET : 소켓 s가 set에 들어가 있으면 0이 아닌값을 리턴한다.

    vector<Session> sessions;
    sessions.reserve(100);

    fd_set reads;
    fd_set writes;

    while (true) {
        //set은 매 루프마다 초기화 시켜준다.
        FD_ZERO(&reads);
        FD_ZERO(&writes);
    
        //FD_SET 은 계속적으로 등록함, 만약에 누군가 호출되면 낙오자는 자연스럽게 제거되기 때문에, 다시 등록 필요.
        //socket 등록
        FD_SET(listenSocket, &reads);

        //socket 등록
        for (Session& s : sessions) {
            //현재는 echo 서버로 작동한다. 만약 recv를 받았으면 보내기를 해야 한다. 그러면 Sendbyte의 크기가 커지기 때문에
            //아래와 같은 if문이 생성된다.
            if (s.recvBytes <= s.sendBytes) 
                FD_SET(s.socket, &reads);
            else
                FD_SET(s.socket, &writes);
        }

        //마지막에 timout인자 설정 가능
        //timeval timeOut;
        //timeOut.tv_sec; //sec
      
        int32 retVal = ::select(0, &reads, &writes, nullptr, nullptr);
        if (retVal == SOCKET_ERROR)
            break;

        //Listen소켓 체크 -> listen 소켓이 준비된 상태이다
        if (FD_ISSET(listenSocket, &reads)) {
            SOCKADDR_IN  clientAddr;
            int32 addrLen = sizeof(clientAddr);
            SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
            if (clientSocket != INVALID_SOCKET) {
                cout << "Client Connected " << endl;
                sessions.push_back( Session{ clientSocket } );
            }
        }

        //나머지 소켓
        for (Session& s : sessions) {
            //Read Check
            if (FD_ISSET(s.socket, &reads)) {
                int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
                if (recvLen <= 0) {
                    //TODO : sessions 제거
                    //연결 끊킨 session은 벡터에서 제거
                    continue;
                }
                s.recvBytes = recvLen;
            }

            //WriteCheck
            if (FD_ISSET(s.socket, &writes)) {
                //Send한 크기 뱉어줌.
                //블로킹 모드 -> 모든 데이터 다 보냄
                //논 블로킹 모드 -> 일부만 보낼 수 있음 (상대방 수신 버퍼 사황에 따라 ) <- 만약 100byte 보낼려는데, 상대방이 받을상황이
                //안된다면 10byte만 갈 수 있다.
                int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
                if (sendLen == SOCKET_ERROR) {
                    //TODO. Session 제거 
                    continue;
                }

                s.sendBytes += sendLen;
                if (s.recvBytes == s.sendBytes) {
                    //0, 0 초기화
                    s.recvBytes = 0;
                    s.sendBytes = 0;
                }
            }
        }

    }

    //원속 종료
    ::WSACleanup();
    return 0;
}

