#pragma once
#include "NetAddress.h"
#include "IocpCore.h"
#include "Listener.h"
#include <functional>

enum class ServiceType : uint8 
{
	Server,
	Client,
};

using SessionFactory = function<SessionRef(void)>;

/// <summary>
/// Server, Client 등.. 여러 정책의 Service클래스
/// </summary>
class Service : public enable_shared_from_this<Service>
{
public:
	Service(ServiceType type, NetAddress netAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~Service();

	virtual bool Start() abstract; //Start;
	bool CanStart() { return _sessionFactory != nullptr; }

	virtual void CloseService();
	void SetSessionFactory(SessionFactory func) { _sessionFactory = func; }

	SessionRef	CreateSession();
	void		AddSession(SessionRef session);
	void		ReleaseSession(SessionRef session);
	int32		GetCurrentSessionCount() { return _sessionCount; }
	int32		GetMaxSessionCount() { return _maxSessionCount; }

public:
	ServiceType GetServiceType() { return _type; }
	NetAddress GetNetAddress() { return _netAddress; }
	IocpCoreRef& GetIocpCore() { return _iocpCore; }


protected:
	USE_LOCK;

	ServiceType _type;
	NetAddress _netAddress = {};
	IocpCoreRef _iocpCore;

	Set<SessionRef> _sessions;
	int32 _sessionCount = 0;
	int32 _maxSessionCount = 0;

	SessionFactory	_sessionFactory;
};

///
/// ClientService 
/// 
/// 
/// 
class ClientService : public Service
{
public:
	ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ ClientService(){ }

	virtual bool Start() override;
	virtual void CloseService();
};


/// 
/// ServerService
/// 

class ServerService : public Service
{
public:
	ServerService(NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ServerService() { }

	virtual bool Start() override;
	virtual void CloseService();
private:
	ListenerRef _listener = nullptr;
};