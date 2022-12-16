#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"

void GameSession::OnConnected()
{
    GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
    GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
    cout << "OnRecv Len = " << len << endl;

    //미리 보낼 바이트를 예측하지 못할때가 생김. 그래서 데이터 크게 잡음
    //그래서 데이터를 보낼때 1byte만 보낼때도 4096을 잡는다.
    //SendBufferRef sendBuffer = MakeShared<SendBuffer>(4096);
    SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
    ::memcpy(sendBuffer->Buffer(), buffer, len);
    sendBuffer->Close(len);

    GSessionManager.BroadCast(sendBuffer);
    return len;
}

void GameSession::OnSend(int32 len)
{
    cout << "OnSend Len " << len << endl;
}