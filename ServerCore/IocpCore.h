#pragma once

/// <summary>
/// 
/// </summary>
/// 자기 스스로 내부에서 shared_pointer를 추출하기 위해선 특수 클래스를 상속바당야 한다.
/// 자기 자신의 weekpoint를 들고있다.
class IocpObject : public enable_shared_from_this<IocpObject>
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