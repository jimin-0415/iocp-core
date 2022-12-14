#pragma once

/// <summary>
/// 
/// </summary>
class IocpObject 
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numofBytes) abstract;

};

/// <summary>
/// Iocp Core
/// </summary>
class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return _iocpHandler; }

	bool Register(class IocpObject* iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE); //worker 쓰레드에서 확인하는 함수 //getQueueCompletionStatuon 
private:
	HANDLE _iocpHandler;
};

//Temp
extern IocpCore GIocpCore;