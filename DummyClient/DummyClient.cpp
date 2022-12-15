#include "pch.h"
#include <iostream>
#include "Service.h"
#include "Session.h"
#include "ThreadManager.h"
using namespace std;

char sendBuffer[] = "Hello World";

class DummySession : public Session {
public :
	~DummySession() {
		cout << "~DummySession" << endl;
	}

	//Override Function To Contents 
	virtual void OnConnected() override 
	{
		cout << " Connected To Server" << endl;
		Send((BYTE*)sendBuffer, sizeof(sendBuffer));
		
	}
	virtual int32	OnRecv(BYTE* buffer, int32 len) override 
	{ 
		cout << "On Recv Len = " << len << endl;
		this_thread::sleep_for(1s);
		Send((BYTE*)sendBuffer, sizeof(sendBuffer));
		return len; 
	}
	virtual void	OnSend(int32 len)override 
	{
		cout << "OnSend Len = " << len << endl;
	}
	virtual void	OnDisconnected()override 
	{ 
		cout << "DisConnected" << endl;
	}
};

int main()
{
	this_thread::sleep_for(1s);

	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<DummySession>,
		1);	//DummyClinet 1개.
	
	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 2; i++) {
		GThreadManager->Launch([=]() {
			while (true) {
				service->GetIocpCore()->Dispatch();
			}
			});
	}
	return 0;
}

