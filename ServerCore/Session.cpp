#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session()
    :_recvBuffer(BUFFER_SIZE)
{
    _socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
    SocketUtils::Close(_socket);
}

//언제 어디서 Send가 호출될지 모른다. 중복이 가능, 몬스터 잡을경우, 귓속말.. 등등..
void Session::Send(BYTE* buffer, int32 len)
{
    //생각할 문제-> 
    //1. Buffer 관리
    //2. SnedEvent 관리, 단일 여러개 WSASend 중첩될지?
    
    //Temp
    SendEvent* sendEvent = xnew<SendEvent>();
    sendEvent->owner = shared_from_this();
    //Send 여러번 호출하면 기존에 영역을 덮어쓸 수 있다.
    //[    ][다음 버퍼] 다음 버퍼부터 밀어넣어야 한다. 
    //Circular Buffer. <- 별로 안좋다. = 이유 : 복사 비용이 존재함.
    //BroadCasting 할때마다. 모든 세션에 복사 마다 별로 좋지 않다.

    sendEvent->buffer.resize(len);
    ::memcpy(sendEvent->buffer.data(), buffer, len);

    WRITE_LOCK;
    //Buffer Pool이 꽉차서 Pending 됬을때도..Lock걸릴경우.
    //Send를 할때마 한번씩 보내는것보다, Send를 호출하면 Buffer Pool을 모았다가 한번에 보내는게 성능상 좋다.
    RegisterSend(sendEvent);
}

bool Session::Connect()
{
    return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
    //한번만 호출한다.
    if (_connected.exchange(false) == false)
        return;

    cout << "Disconnect: " << cause << endl;

    //Contents 단 Override
    OnDisconnected();
    GetService()->ReleaseSession(GetSessionRef());

    RegisterDisConnect();
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
        //Connector 라는 클래스로 따로 빼는것도 좋을거 같다.
        //Client가 우리 서버로 붙을경우 Seesion 만들어질경우 
        ProcessConnect();
        break;
    case EventType::DisConnect:
        ProcessDisconnect();
        break;
    case EventType::Recv:
        ProcessRecv(numOfBytes);
        break;
    case EventType::Send:
        ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfBytes);
        break;
    default:
        break;
    }
}

bool Session::RegisterConnect()
{
    //두번 연결할 필요 x
    if (IsConnected())
        return false;

    //Client 서비스에서만 가능 
    if (GetService()->GetServiceType() != ServiceType::Client)
        return false;
    
    if (SocketUtils::SetReuseAddress(_socket, true) == false)
        return false;

    if (SocketUtils::BindAnyAddress(_socket, 0 /*남는 주소 아무거나*/) == false)
        return false;

    _connectEvent.Init();
    _connectEvent.owner = shared_from_this();   //Add _ref

    DWORD numOfBytes = 0;
    SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();
    if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr
        , 0, &numOfBytes, &_connectEvent)) {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING) {
            _connectEvent.owner = nullptr; //Release ref
            return false;
        }
    }
    return true;
}

bool Session::RegisterDisConnect()
{
    _disConnectEvent.Init();
    _disConnectEvent.owner = shared_from_this();    //Add ref
    
    //DE_REUSE_SOCKET : 소켓을 재사용한다. <- AcceptEx, ConnectEx 에 소켓을 재사용하도록 넘긴다.// IOCP로 전달
    if (false == SocketUtils::DisconnectEx(_socket, &_disConnectEvent, TF_REUSE_SOCKET, 0)) {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING) {
            _disConnectEvent.owner = nullptr; //release ref
            return false;
        }
    }
    return true;
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

    //TCP는 바운더리 개념이 없기 때문에 .짤려서 넘겨올 수 있다. 패킷을 모두 받아야 처리가 가능하다.
    WSABUF wsaBuf;

    //Write Pos 위치에 기록한다.
    wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
    wsaBuf.len = _recvBuffer.FreeSize();   //최대 버퍼보다 크게 잡는다.

    DWORD numOfBytes = 0;   //수신버퍼에서 받아온 실제 사이즈
    DWORD flags = 0;

    if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT &numOfBytes, OUT &flags, &_recvEvent, nullptr)) {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING) {
            HandleError(errorCode);
            _recvEvent.owner = nullptr; //레퍼런스 카운트 1 감소
        }
    }
}

void Session::RegisterSend(SendEvent* sendEvent)
{
    if (IsConnected() == false)
        return;

    WSABUF wsaBuf;
    wsaBuf.buf = (char*)sendEvent->buffer.data();
    wsaBuf.len = (ULONG)sendEvent->buffer.size();

    DWORD numofBytes = 0;
    //WSASend가 Thread Safe 한가 ? Thread Safe하지 않는다.

    if (SOCKET_ERROR == ::WSASend(_socket, &wsaBuf, 1, OUT & numofBytes, 0, sendEvent, nullptr)) {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING) {
            HandleError(errorCode);
            sendEvent->owner = nullptr; //Release Ref
            xdelete(sendEvent);
        }
    }
}

void Session::ProcessConnect()
{
    _connectEvent.owner = nullptr; //release ref

    _connected.store(true);

    //Session 등록
    GetService()->AddSession(GetSessionRef());

    //컨텐츠코드 오버로딩
    OnConnected();

    //수신 등록 
    RegisterRecv();

    
}

void Session::ProcessDisconnect()
{
    //할게 없다.
    _disConnectEvent.owner = nullptr; //release ref Owner;
}

void Session::ProcessRecv(int32 numOfBytes)
{
    _recvEvent.owner = nullptr; //Release Ref Owner 에 대한 생명 주기 관리를 위해서 nullptr 로 날려준다.
    if (numOfBytes == 0) {
        Disconnect(L"Recv 0");
        return;
    }

    //Write Pos을 numOfBytes만큼 땡긴다
    if (_recvBuffer.OnWrite(numOfBytes) == false) {
        Disconnect(L"OnWrite OverFlow");
    }

    int32 dataSize = _recvBuffer.DataSize();
    
    //Contents Override <- 이만큼 읽었다는것을 컨텐츠에 넘겨서 처리를 시킨다.
    //처리 개수를 받는다.
    int32 processLen = OnRecv(_recvBuffer.ReadPos(), numOfBytes);
    //ReadCur앞으로 땡기기.
    if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false) {
        Disconnect(L"OnRead OverFlow");
        return;
    }

    //커서 정리. 초기화 여부 확인
    _recvBuffer.Clean();

    //수신 등록
    RegisterRecv();
}

//Send는 Recv처럼 거는게 아니라... 넘길 데이터가 있을경우 Send를 호출한다. 중복 호출일경우 고려해야함
void Session::ProcessSend(SendEvent* sendEvent, int32 numOfBytes)
{
    sendEvent->owner = nullptr; //release event
    xdelete(sendEvent);

    if (numOfBytes == 0) {
        Disconnect(L"Send 0");
        return;
    }

    //컨텐츠 코드에서 오버로딩 -> ProcessSend는 순서가 보장되지 않느다.
    OnSend(numOfBytes);
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
