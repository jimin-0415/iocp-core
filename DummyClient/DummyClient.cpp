#include "pch.h"
#include <iostream>
#include "Service.h"
#include "Session.h"
#include "ThreadManager.h"
#include "BufferReader.h"
#include "ClientPacketHandler.h"
using namespace std;

char sendData[] = "Hello World";

class DummySession : public PacketSession {
public :
	~DummySession() {
		cout << "~DummySession" << endl;
	}

	//Override Function To Contents 
	virtual void OnConnected() override 
	{
		//cout << " Connected To Server" << endl;

		/*SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
		::memcpy(sendBuffer->Buffer(), sendData, sizeof(sendData));
		sendBuffer->Close(sizeof(sendData));
		
		Send(sendBuffer);*/
	}
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override 
	{ 
		//이미 Packet이 계산이 되서 왔기 때문에 len을 다시 보내줄 필요가 없다.
		ClientPacketHandler::HandlePacket(buffer, len);
	}
	virtual void OnSend(int32 len)override 
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
	this_thread::sleep_for(1ms);

	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<DummySession>,
		1);	//DummyClinet 5개.
	
	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 5; i++) {
		GThreadManager->Launch([=]() {
			while (true) {
				service->GetIocpCore()->Dispatch();
			}
			});
	}

	GThreadManager->Join();
	return 0;
}

