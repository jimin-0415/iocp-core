#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"
//Temp
IocpCore GIocpCore;

IocpCore::IocpCore()
{
	_iocpHandler = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandler != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandler);
}

bool IocpCore::Register(IocpObject* iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandler, reinterpret_cast<ULONG_PTR>(iocpObject), 0);
}

//worker Thread가 확인 하는 함수.
bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numofBytes = 0;
	IocpObject* iocpObject = nullptr;	//등록한 iocpObject와 
	IocpEvent* iocpEvent = nullptr;		//어떤 이벤트가 호출됬는지 아는 iocpEvent를 out으로 보냄
	
	if (::GetQueuedCompletionStatus(_iocpHandler, OUT & numofBytes, OUT reinterpret_cast<PULONG_PTR>(&iocpObject)
		, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs)) {

		iocpObject->Dispatch(iocpEvent, numofBytes);

	}
	else {
		int32 errorCode = ::WSAGetLastError();
		switch (errorCode) {
		case WAIT_TIMEOUT:
			//타임 아웃은 에러가 아님,
			return false;
		default:
			//TODO : 로그 찍기.
			iocpObject->Dispatch(iocpEvent, numofBytes);
			break;
		}
	}
	
	return true;
}
