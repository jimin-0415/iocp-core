#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session()
{
    _socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
    SocketUtils::Close(_socket);
}

void Session::Disconnect(const WCHAR* cause)
{
    //기존의 값을 뱉어준다.
    if (_connected.exchange(false) == false)
        return;

    cout << "Disconnect: " << cause << endl;

    //Contents 단 Override
    OnDisconnected();
    SocketUtils::Close(_socket);
    GetService()->ReleaseSession(GetSessionRef());
}

HANDLE Session::GetHandle()
{
    return reinterpret_cast<HANDLE>(_socket);
}

//Iocp 통지
void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
    switch (iocpEvent->eventType) {
    case EventType::Connect:
        ProcessConnect();
        break;
    case EventType::Recv:
        ProcessRecv(numOfBytes);
        break;
    case EventType::Send:
        ProcessSend(numOfBytes);
        break;
    default:
        break;
    }
}

void Session::RegisterConnect()
{
}

void Session::RegisterRecv()
{
    if (IsConnected() == false)
        return;

    //계속 만들고, 삭제하고 반복
    //1. Session마다 실행을 할때 refCount 증가 용도로 활용
    //2. IocpPort에서 어뻔 오버랩인지 인지하는 용도로 사용하는데,
    //굳이 만드는것보다는 맴버변수로 가지고 사용하는게 좋다.
    //RecvEvent* recvEvent = xnew<RecvEvent>();   
    _recvEvent.Init();
    _recvEvent.owner = shared_from_this();  //레퍼런트 카운트 1 증가.

    WSABUF wsaBuf;
    wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer);
    wsaBuf.len = len32(_recvBuffer);

    DWORD numOfBytes = 0;
    DWORD flags = 0;

    if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT &numOfBytes, OUT &flags, &_recvEvent, nullptr)) {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING) {
            HandleError(errorCode);
            _recvEvent.owner = nullptr; //레퍼런스 카운트 1 감소
        }
    }
}

void Session::RegisterSend()
{
}

void Session::ProcessConnect()
{
    _connected.store(true);

    //Session 등록
    GetService()->AddSession(GetSessionRef());

    //컨텐츠코드 오버로딩
    OnConnected();

    //수신 등록
    RegisterRecv();

    
}

void Session::ProcessRecv(int32 numOfBytes)
{
    _recvEvent.owner = nullptr; //Release Ref Owner 에 대한 생명 주기 관리를 위해서 nullptr 로 날려준다.
    if (numOfBytes == 0) {
        Disconnect(L"Recv 0");
        return;
    }

    //TODO
    cout << "Recv Data Len = " << numOfBytes << endl;

    //수신 등록
    RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
}

void Session::HandleError(int32 errorCode)
{
    switch (errorCode) {
    case WSAECONNRESET:
    case WSAECONNABORTED:
        Disconnect(L"Handle Error");
        break;
    default:
        //TODO Log.
        cout << " Handle Error : " << errorCode << endl;
        break;
    }
}

void Session::OnConnected()
{
}
