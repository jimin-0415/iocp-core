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

void Session::Send(SendBufferRef sendBuffer)
{
    //WriteLock으로 걸게되면 좀 비효율 적인
    if (IsConnected() == false)
        return;

    bool registerSend = false;

    //현재 RegisterSend가 걸리지 않았다면 건다.
    {
        WRITE_LOCK;
        _sendQueue.push(sendBuffer);
        if (_sendRegistered.exchange(true) == false) {
            //Send할경우 계속적으로 보내는것보다 Recv처럼 WSASend가 끝나서 통지를 받아서 순차저긍로 하는것이 좀더 성능상 이점이있다.
            //System Call 횟수가 줌.
            registerSend = true;
        }
    }

    if (registerSend)
        RegisterSend();
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
        ProcessSend(numOfBytes);
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

void Session::RegisterSend()
{
    if (IsConnected() == false)
        return;

    //iocp 에 SendEvent 호출할준비.
    _sendEvent.Init(); 
    _sendEvent.owner = shared_from_this(); //add ref

    //하나의 버퍼를 보내는게 아니라 Queeu에 데이터를 쌓은것을 보낸다.
    /*WSABUF wsaBuf;
    wsaBuf.buf = (char*)_sendEvent.buffer.data();
    wsaBuf.len = (ULONG)_sendEvent.buffer.size();*/

    //보낼 데이터를 SendEvent에 등록
    {
        WRITE_LOCK;
        //Queue에서 꺼내는 순간 Ref count down -> 보낼 데이터를 Evenet에 등록
        int32 writeSize = 0;
        while (_sendQueue.empty() == false) {
            SendBufferRef sendBuffer = _sendQueue.front();

            writeSize += sendBuffer->WriteSize();
            //TODO : 예외 체크

            _sendQueue.pop();
            _sendEvent.sendBuffers.push_back(sendBuffer);
        }
    }

    //Scatter-Gather [흩어져 있는 데이터들을 모아서 한번에 보낸다.]
    Vector<WSABUF> wsaBufs;
    wsaBufs.reserve(_sendEvent.sendBuffers.size());
    for (SendBufferRef sendBuffer : _sendEvent.sendBuffers) {
        WSABUF wsaBuf;
        wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
        wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
        wsaBufs.push_back(wsaBuf);
    }

    DWORD numofBytes = 0; 
    if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numofBytes, 0, &_sendEvent, nullptr)) {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING) {
            HandleError(errorCode);
            _sendEvent.owner = nullptr; //Release Ref
            _sendEvent.sendBuffers.clear(); //Release Buffer
            _sendRegistered.store(false);   //다음사람이 호출 가능하게 
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

    //Contents 단 Override
    OnDisconnected();
    GetService()->ReleaseSession(GetSessionRef());

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
void Session::ProcessSend(int32 numOfBytes)
{
    _sendEvent.owner = nullptr; //release event
    _sendEvent.sendBuffers.clear(); //SendBuffer를 모두 Clear 한다. 모두 사용 완료했기 때문에.

    if (numOfBytes == 0) {
        Disconnect(L"Send 0");
        return;
    }

    //컨텐츠 코드에서 오버로딩 -> ProcessSend는 순서가 보장되지 않느다.
    OnSend(numOfBytes);

    WRITE_LOCK; //Send하는 사람마다 SendQueue에 넣고 있기 때문에 WriteLock 걸어준다.
    if (_sendQueue.empty()) {
        _sendRegistered.store(false);   //다음사람이 호출하면 바로 IOCP에 등록할 수 있도록한다.
    }
    else {  //이미 Queue에 쌓여있다. Register 를 바로 호출한다.
        RegisterSend();
    }
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

PacketSession::PacketSession()
{
}

PacketSession::~PacketSession()
{
}

//[size][id][data...  ] [size][id][data...]
int32 PacketSession::OnRecv(BYTE* buffer, int32 len)
{
    int32 processLen = 0;
    while (true) {
        //일반적인 데이터 사이즈
        int32 dataSize = len - processLen;
        //최소한 PacketHeader 파싱 가능한가?

        if (dataSize < sizeof(PacketHeader))
            break;

        PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
        
        //Header에 기록된 패킷 크기 파싱 가능한가 ?
        if (dataSize < header.size)
            break;
        
        //패킷 조립 가능
        OnRecvPacket(&buffer[0], header.size);
        processLen += header.size;
    }

    return processLen;
}

int32 PacketSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    return int32();
}
