#pragma once
#include <stack>
#include <map>
#include <vector>

/// <summary>
/// Deaf Lock 상태를 확인한다.
/// Lock 행할 경우 Dead Lock 이 발생되는지 확인
/// Debug 모드에서만 확인한다.
/// </summary>

class DeadLockProfiler
{
public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(int32 index);

private:
	unordered_map<const char*, int32>	_nameToId;	//생성된 Lock 개수
	unordered_map<int32, const char*>	_idToName;


	// 0 -> 1 -> 2
	// 3 -> 1 -> 5  각 개별 락 스택이 다른데 해당 부분 빼지 않았음. TLS로 뺴면됨
	map<int32, set<int32>>				_lockHistory;	//그래프의 간선 집합 1번락을 잡고 4번 락을 작을려고 할때, 한번도 해당 Lock에 대해서 검증을 하지 않았다면 cycle 검증

	Mutex _lock;

//순참 탐색을 위한 데이터
private:
	vector<int32> _discoveredOrder;		//Node 발견된 순서 기록 배열 
	int32 _discoverdCount = 0;			//Node 발견 순서
	vector<bool> _finished;				//Dfs(i)종료 여부
	vector<int32> _parent;
};

