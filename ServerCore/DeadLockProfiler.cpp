#include "pch.h"
#include "DeadLockProfiler.h"

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	int32 lockId = 0;
	auto findId = _nameToId.find(name);
	if (findId == _nameToId.end()) {
		//Lock Id 0 부터 발급
		lockId = static_cast<int32>(_nameToId.size());	
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else {
		lockId = findId->second;
	}
	
	//LockStack check
	if (_lockStack.empty() == false) {
		//기존 발견되지 않은 케이스라면 데드락 여부 확인.
		const int32 prevId = _lockStack.top();
		if (lockId != prevId) {
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end()) {
				//history에 넣는다.
				history.insert(lockId);
				CheckCycle();
			}
		}
	}

	_lockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	if (_lockStack.empty())
		CRASH("Multiple Unlock");

	int32 lockId = _nameToId[name];
	if (_lockStack.top() != lockId)
		CRASH("Invalid Unlock");

	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	//Init
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);	
	_discoverdCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; lockId++) {
		Dfs(lockId);
	}

	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	//방문상태
	if (_discoveredOrder[here] != -1)
		return;

	_discoveredOrder[here] = _discoverdCount++;

	//모든 정점을 순회함.
	auto findit = _lockHistory.find(here);
	//내가 따른 락을 잡은 적이 없다.
	if (findit == _lockHistory.end()) {
		_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findit->second;
	for (int32 there : nextSet) {
		//아직 방문한적이 없을 경우
		if (_discoveredOrder[there] == -1) {
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		//here 가 there 보다 먼저 발결될 경우.
		//threr 는 here의 후손이다 [순방향 간선]
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		//순방향이 아니고, Dfs(threr)가 아직 종료하지 않은 경우,
		// there는 here의 선조다 [역방향 간선]
		if (_finished[there] == false) {
			cout << _idToName[here] << " -> " << _idToName[there] << endl;

			int now = here;
			while (true) {
				cout << _idToName[_parent[now]] << " -> " << _idToName[now] << endl;
				now = _parent[now];
				if (now == there)
					break;
			}
			CRASH("Detected DeadLock");
		}
	}

	_finished[here] = true;
}
