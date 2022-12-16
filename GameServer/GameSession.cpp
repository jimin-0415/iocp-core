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

int32 GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[0]));
    
    cout << "Packet ID :" << header.id;
    cout << "Packet Size : " << header.size;
    
    return len;
}

void GameSession::OnSend(int32 len)
{
    cout << "OnSend Len " << len << endl;
}