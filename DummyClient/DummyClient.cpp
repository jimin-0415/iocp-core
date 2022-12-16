#include "pch.h"
#include <iostream>
#include "Service.h"
#include "Session.h"
#include "ThreadManager.h"
#include "BufferReader.h"
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
	virtual int32	OnRecvPacket(BYTE* buffer, int32 len) override 
	{ 
		BufferReader br(buffer, len);
		PacketHeader header;
		br >> header;

		uint64 id;
		uint32 hp;
		uint16 attack;
		br >> id >> hp >> attack;

		cout << "ID : " << id << " HP : " << hp << " Attack :" << attack << endl;

		char recvBuffer[4096];
		//현재는 길이를 이렇게 보내지만 가변길이의 경우 가변길이의 Packet Size를 추가로 보내주면 된다. 일단은 임시로 
		br.Read(recvBuffer, header.size - sizeof(PacketHeader) - sizeof(uint64) - sizeof(uint32) - sizeof(uint16));
		cout << recvBuffer << endl;

		return len; 
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
		500);	//DummyClinet 5개.
	
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

