#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

class Service;

/// <summary>
/// Session
/// </summary>

class Session : public IocpObject
{
	//아래 private Function은 외부에서 접근 못하지만, [Listener, IocpCore, Service] 
	//그렇게 되면 Listener와 Iocp와 같은 클래스에서 접근 못하니 friend 로 접근 가능하게 만들어준다.
	friend class Listener;
	friend class IocpCore;
	friend class Service;
public:
	Session();
	virtual ~Session();

public:
	void	Send(BYTE* buffer, int32 len);
	void	Disconnect(const WCHAR* cause);	//해킹 의심 , 상대방 연결 끊킴

	shared_ptr<Service> GetService() { return _service.lock(); }
	void	SetService(shared_ptr<Service> service) { _service = service; }

public:
	//Info Funtion
	void SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress GetNetAddress() { return _netAddress; }
	SOCKET	GetSocket() { return _socket; }
	bool IsConnected() { return _connected; }
	SessionRef GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

private:
	//interface
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes) override;

private:
	//전송 관련 Protocol
	void	RegisterConnect();
	void	RegisterRecv();
	void	RegisterSend(SendEvent* sendEvent);

	void	ProcessConnect();
	void	ProcessRecv(int32 numOfBytes);
	void	ProcessSend(SendEvent* sendEvent, int32 numOfBytes);

	void	HandleError(int32 errorCode);

protected:
	//Override Function To Contents 
	virtual void	OnConnected();
	virtual int32	OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void	OnSend(int32 len) { }
	virtual void	OnDisconnected() { }

public:
	//Temp
	BYTE _recvBuffer[1000];

private:
	//내부에서 서비스에 대한 정보를 알아야 한다.
	weak_ptr<Service> _service;
	SOCKET _socket = INVALID_SOCKET;
	NetAddress _netAddress = {};
	Atomic<bool> _connected = false;	//접속 여부

private:
	USE_LOCK;

	//수신 기능

	//송신 기능

private:
	//IocpEvent 재사용..
	RecvEvent	_recvEvent;
};

