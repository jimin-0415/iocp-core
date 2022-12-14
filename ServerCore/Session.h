#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

/// <summary>
/// Session
/// </summary>

class Session : public IocpObject
{
public:
	Session();
	~Session();

public:
	//public functrion
	void SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress GetNetAddress() { return _netAddress; }
	SOCKET	GetSocket() { return _socket; }
public:
	//interface 
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numofBytes) override;
public:
	
	//Temp
	char _recvBuffer[1000] = {};

private:
	SOCKET _socket = INVALID_SOCKET;
	NetAddress _netAddress = {};
	Atomic<bool> _connected = false;	//접속 여부
};

