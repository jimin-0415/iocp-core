#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents) {

		xdelete(acceptEvent);
	}
}

bool Listener::StartAccept(ServerServiceRef service)
{
	_service = service;
	if (_service == nullptr)
		return false;
	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;
	
	//iocp 등록 <-Listen 소켓 iocp 등록    //smart Pointer 로 변환.
	if (_service->GetIocpCore()->Register(shared_from_this()) == false)
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	if (SocketUtils::Bind(_socket, _service->GetNetAddress()) == false)
		return false;
	
	if (SocketUtils::Listen(_socket) == false)
		return false;

	//accept 등록
	const int32 acceptCount = _service->GetMaxSessionCount();
	for (int i = 0; i < acceptCount; i++) {
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();
		acceptEvent->owner = shared_from_this();	//ref 는 유지한채로 자기 자신을 넣는다. //acceptEvent->owner = shared_ptr<IocpObject>(this); <- 이렇게 하면 안된다.
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return true;
}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE> (_socket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numofBytes)
{
	ASSERT_CRASH(iocpEvent->eventType == EventType::Accept);
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

//iocp에서 처리할 수 있도록 accept를 등록한다
void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	//AcceptEvent에 해당 Session정보를 바인딩 해준다.
	SessionRef session = _service->CreateSession();	//Register Iocp;
	
	acceptEvent->Init();
	acceptEvent->session = session;
	
	DWORD bytesReceived = 0;

	//당장 실행은 안되지만, Pending상태가 되면 iocp로 부터 오게된다, Listener 소켓을 iocp에 등록했기 때문.
	if (false == SocketUtils::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent))) {
		const int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) // Pending 아닐경우.
		{
			//다시 accpet 건다.
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = acceptEvent->session;
	
	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket)) {
		//문제가 발생하면 RegisterAccept를 걸어줘야한다. 재사용 해야 하기 때문.
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sizeofSockAddr = sizeof(sockAddress);
	//상대방의 peer 정보를 얻는 함수
	if (SOCKET_ERROR == ::getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeofSockAddr)) {
		RegisterAccept(acceptEvent);
		return;
	}

	session->SetNetAddress(NetAddress(sockAddress));
	session->ProcessConnect();
	cout << "Client Connected !" << endl;

	RegisterAccept(acceptEvent);
}
