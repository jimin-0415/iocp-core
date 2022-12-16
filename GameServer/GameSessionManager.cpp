#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"

GameSessionManager GSessionManager;

void GameSessionManager::Add(shared_ptr<GameSession> session)
{
	WRITE_LOCK;
	_sessions.insert(session);
}

void GameSessionManager::Remove(shared_ptr<GameSession> session)
{
	WRITE_LOCK;
	_sessions.erase(session);
}

void GameSessionManager::BroadCast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	//SendBuffer 하나로 모든 Session에 전송한다.
	for (shared_ptr<GameSession> gameSession : _sessions) {
		gameSession->Send(sendBuffer);
	}
}
