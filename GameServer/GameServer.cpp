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

    // WSAEventSelect = (WSAEventSelect 함수가 핵심이다)
    /// 소켓과 관련된 네이트워 이벤트 [이벤트객체] 를 통해서 감지한다.
    /// 
    /// 이벤트 객체 관련 함수들
    /// 생성 : WSACreateEvent() - 수동 리셋 Manual-Resset + Non-Signaled 상태 시작
    /// 삭제 : WSACloseEvent();
    /// 신호 상태 감지 : WSAWaitForMultipleEvents
    /// 구체적인 네트워크 이벤트 알아내기 : WSAEnumNetworkEvents;
    /// 
    /// 1. 소켓 - 이벤트 객체 연동 - 소켓 개수 만큼 이벤트 개수를 만들어야 한다.
    /// WSAEventSelect(socket, event, networkEvent); <- 이녀석은 단순히 등록함.
    /// - 관심있는 네트워크 이벤트 
    /// FD_Accept : 접속 클라가 있음 accept
    /// FD_READ : 데이터 수신 가능 , recv, recvfrom
    /// FD_WRITE : 데이터 송신 가능 , send , sendto
    /// FD_CLOSE : 상대가 접속 종료
    /// FD_CONNECT : 통신을 위한 연결 절차 완료
    /// FD_OOB 
    
    //주의 사항
    // WSAEventSelect 함수를 호출하면, 해당 소켓을 자동으로 넌 블로킹 모드로 전환한다.
    // accept()함수를 리턴하는 소켓은 listenSocket과 동일한 속성을 갖는다.
    // - 따라서 clientSocket은 FD_READ, FD_WRITE 등을 다시 등록해야한다.
    // 드물게 WSAEWOULDBLOCK 오류에 대한 예외 처리가 필요하다.
    // 중요)
    // - 이벤트 발생 시 , 적절한 소켓 함수 호출을 해야 한다.
    // - 아니면 다음번에는 동일 네트워크 이벤트가 발생하지 않는다.
    // 예시 ) 만약 FD_READ를 탐색도중 event 호출이 되서 FD_READ에 .. recv 를 하지 않을 경우, 다시 통지 안해줌
    // 이벤트가 발생하면 짝이 맞는 녀석에 대해서 호출을 해야 한다. - 안하면 두번다시 이벤트 호출되지 않음

    // 1). count , event
    // 2) waitAll : 모두 기다림? 하나만 완료되도 OK?
    // 3) timeout : 타임아웃 시간
    // 4) 지금은 사용 x
    // return : 완료된 첫번째 인덱스
    //WSAWaitForMultipleEvents. <- 이걸로 이벤트 호출을 받는다.

    //1 ) socket
    // 2) eventObject : socket과 연동된 이벤트 객체 핸들을 넘겨주면, 이벤트 객체를 non-signaled 상태로 변경
    // 3) networkEvent : 네트워크 이벤트 / 오류정보가 저장
    //WASEnumNetworkEvents;

    vector<WSAEVENT> wsaEvents;
    vector<Session> sessions;
    sessions.reserve(100);

    WSAEVENT listenEvent = ::WSACreateEvent();
    wsaEvents.push_back(listenEvent);
    sessions.push_back(Session{ listenSocket });

    //Accept close의 event 를 받는다.
    if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
        return 0;

    while (true) 
    {
        this_thread::sleep_for(1s);

        //WSA_WAIT_EVENT_0 더한값이 outPut으로 나옴 MSDNS에 나옴
        //100개의 이벤트를 모두 등록 <- 동일하게, select처럼 이벤트개수 64개 제약이 있음
        int32 index =::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
        if (index == WSA_WAIT_FAILED)
            continue;

        index -= WSA_WAIT_EVENT_0;

        //이벤트 초기화 <- 해당 기능은 WSAEnumNetworkEvents 여기에 포함되서 안해도 상관없다.
        //::WSAResetEvent(wsaEvents[index]);

        //어떤 애들때문에 event가 발생했는지 모른다 그걸 체크하기 위해 아래 함수 사용
        //관찰할려는 결과물이 아래 구조체에 채워짐 -> 결과 물 추출했다.
        WSANETWORKEVENTS netWorkEvents;
        if (::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &netWorkEvents) == SOCKET_ERROR)
            continue;
           
        //Listner 소켓 체크
        if (netWorkEvents.lNetworkEvents & FD_ACCEPT) {
            //error-check
            if (netWorkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
                continue;

            SOCKADDR_IN  clientAddr;
            int32 addrLen = sizeof(clientAddr);
            SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
            if (clientSocket != INVALID_SOCKET) {
                cout << "Client Connected " << endl;
                
                //새로 생성된 소켓을 다시 Event에 바인딩 한다.
                WSAEVENT clientEvent = ::WSACreateEvent();
                wsaEvents.push_back(clientEvent);
                sessions.push_back(Session{ clientSocket });
                
                //등록
                if (::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
                    return 0;
            }
        }

        if (netWorkEvents.lNetworkEvents & FD_READ || netWorkEvents.lNetworkEvents & FD_WRITE) {
            //error-check Read
            if ((netWorkEvents.lNetworkEvents & FD_READ) && (netWorkEvents.iErrorCode[FD_READ_BIT] != 0))
                continue;

            //error-check Write
            if ((netWorkEvents.lNetworkEvents & FD_WRITE) && (netWorkEvents.iErrorCode[FD_WRITE_BIT] != 0))
                continue;

            Session& s = sessions[index];

            if (s.recvBytes == 0) {
                int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
                if (recvLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK) {
                    //TODO : Revmove Session
                    continue;
                }

                s.recvBytes = recvLen;
                cout << "Recv Data = " << recvLen << endl;
            }
            
            //Write
            if (s.recvBytes > s.sendBytes) {
                //한버에 안보내주는 경우는 없지만 그래도 안전하게 만들어야 한다.
                int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
                if (sendLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK) {
                    //TODO : Remove Session
                    continue;
                }

                s.sendBytes += sendLen;
                if (s.recvBytes == s.sendBytes) {
                    //모두 받았으면 다시 초기화
                    s.recvBytes = 0;
                    s.sendBytes = 0;
                }

                cout << "Send Data = " << sendLen << endl;
            }
        }

        if (netWorkEvents.lNetworkEvents & FD_CLOSE) {
            //TODO : Close 
        }
    }

    //원속 종료
    ::WSACleanup();
    return 0;
}

