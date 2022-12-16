#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"
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
	
	enum {
		BUFFER_SIZE = 0x10000, //64kb
	};

public:
	Session();
	virtual ~Session();

public:
	void    Send(SendBufferRef sendBuffer);
	bool	Connect();
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
	bool	RegisterConnect();
	bool	RegisterDisConnect();
	void	RegisterRecv();
	void	RegisterSend();

	void	ProcessConnect();
	void	ProcessDisconnect();
	void	ProcessRecv(int32 numOfBytes);
	void	ProcessSend(int32 numOfBytes);

	void	HandleError(int32 errorCode);

protected:
	//Override Function To Contents 
	virtual void	OnConnected();
	virtual int32	OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void	OnSend(int32 len) { }
	virtual void	OnDisconnected() { }

public:
	//Temp
	//Recv 의 경우는 멀티 쓰레드 환경을 고려하지 않아도 된다.
	//1. 한번 요청하면 하나의 쓰레드가 recv 에 대한 처리를 하기 때문에
	

private:
	//내부에서 서비스에 대한 정보를 알아야 한다.
	weak_ptr<Service> _service;
	SOCKET _socket = INVALID_SOCKET;
	NetAddress _netAddress = {};
	Atomic<bool> _connected = false;	//접속 여부

private:
	USE_LOCK;
	//수신 버퍼
	RecvBuffer	_recvBuffer;
	//송신 버퍼
	//Regist Send를 실행중이라서 WSA Send를 할 수 없다면 Queeu에 넣는다.
	Queue<SendBufferRef> _sendQueue;
	Atomic<bool>		_sendRegistered = false;
private:
	//IocpEvent 재사용..
	RecvEvent	_recvEvent;
	SendEvent	_sendEvent;
	//IOCPEvent 재사용.. ServerType::Client 에서 사용
	ConnectEvent _connectEvent;
	DisConnectEvent _disConnectEvent;
};

///
/// PacketSession 
/// 받는곳과 관련있다.
/// OnRecv를 통해서 현재 얼마큼 받았는지 Length 값을 내보내고 있다 [어디까지 처리햇는지에 대한 정보]
/// 만약 Packet이 많이 몰리게 될 경우 TCP 특성상 , 내쪽에서 보내는 모든 패킷이 한번에 보내지지 않는다, 데이터바운드가 없음, 100byte만 보내면 20byte씩 쪼개질 수 있다.
/// 어떤 식으로든 모든 데이터가 전송됬는지 확인하는게 필요하다.
/// 1. 방법 : 특수 문자열로 하면 되지 않을까 ? - 만약 다른 문자열이 겹쳐버리면 문제가 생김.
/// 2. PacketHeader 를 만들어서 정보를 넘겨준다.
/// 

//   header
//[size][id][data...  ]
struct PacketHeader 
{
	uint16 size;	//String 같은 가변 데이터를 보내야한다, 혹은 동적배열.	
	uint16 id;	//ProtocolID -> 1. Login , 2.이동 요청
};

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

	
	PacketSessionRef GetPacketSessionRef() { return static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	virtual int32	OnRecv(BYTE* buffer, int32 len) sealed;	//PacketSession을 상속받은 애들은 해당 함수 사용 못함.
	virtual int32	OnRecvPacket(BYTE* buffer, int32 len) abstract;


};

