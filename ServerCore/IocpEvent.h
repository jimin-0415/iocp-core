#pragma once
class Session;

enum class EventType : uint8
{
	Connect,
	Accept,
	Recv,
	Send,
};
/// <summary>
/// Overlapped
/// </summary>
class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);	//virtual 로 만들지 않는 이유, 가상함수 테이블이 있을경우 첫번째 메모리가 Overlapped로 사용 불가
	
	void Init();

public:
	EventType eventType;
	IocpObjectRef owner;
};

/// <summary>
/// Connect Event 
/// </summary>
class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent()
		:IocpEvent(EventType::Connect) {

	}
private:
	//TODO. 인자가 추가로 있을수도 없을 수 있다.
};


/// <summary>
/// Accept Event 
/// </summary>
/// 
class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent()
		:IocpEvent(EventType::Accept) { }

public:
	//TODO. Accept시 발급 할 Session
	SessionRef session = nullptr;
};


/// <summary>
/// Recv Event 
/// </summary>
class RecvEvent : public IocpEvent
{
public:
	RecvEvent()
		:IocpEvent(EventType::Recv) {

	}
private:
	//TODO. 인자가 추가로 있을수도 없을 수 있다.
};

/// <summary>
/// Send Event 
/// </summary>
class SendEvent : public IocpEvent
{
public:
	SendEvent()
		:IocpEvent(EventType::Send) {

	}
public:
	vector<BYTE> buffer;
	//TODO. 인자가 추가로 있을수도 없을 수 있다.
};