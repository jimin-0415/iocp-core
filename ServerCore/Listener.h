#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;

/// <summary>
/// Listen Session
/// </summary>
 
class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

public:
	//public Function
	bool StartAccept(NetAddress netAddress);
	void CloseSocket();

public:
	//interface
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numofBytes) override;

private:
	//수신 관련 코드
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);
	
protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;
};

