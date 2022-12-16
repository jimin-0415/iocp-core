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


//BroadCast 도중 Client 강제 종료시 크래시 발생
//1 Ref는 수동으로 관리하는게 아닌 자동으로 관리하기 때문에 x
//2 Lock 의 문제도 의심. Mutex로 바꿔서 실행해도 문제가 안생심, DeadLock x

void GameSessionManager::BroadCast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	//SendBuffer 하나로 모든 Session에 전송한다.
	for (shared_ptr<GameSession> gameSession : _sessions) {
		gameSession->Send(sendBuffer);

		//중간에 COnnecttion이 끊키게 되면 여러 함수 타서 결국엔 _session.erase(session);가 호출될 수 있다.
		//먼가 재귀적으로 Session을 건들여서 그럼.
		//실제 Session이 DisConnect 될때 현재 쓰레드가 아닌 다른 쓰레드가 호출되도록 해준다.
	}
}
