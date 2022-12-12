# 공부 자료 파일 입니다.

### Thread 관련 공부 자료
```
#include "pch.h"

//C++11 threda linux and windows
#include <thread> 

void HelloThread() {
	cout << "Hello Thread" << endl;
}

void HelloThread_2(int32 num) {

}

int main()
{
	//System Call (OS 커널에 요청)
	cout << "hello world" << endl;

	std::thread t(HelloThread);
	std::thread t2(HelloThread_2, 2);

	/// <summary>
	/// CPU 코어의 개수는 몇개인가? 그것을 추출하는 정보
	/// </summary>
	/// <returns></returns>
	t.hardware_concurrency();

	/// <summary>
	/// 각 Thraed 마다 부여되는 ID , ID 중복 불가는 보장한다.
	/// </summary>
	/// <returns></returns>
	t.get_id();

	/// <summary>
	/// std 쓰레드 객체에서 실제 쓰레드를 분리한다.
	/// 대신 분리를 하면 해당 쓰레드를 직접 관리가 불가능해진다.
	/// </summary>
	/// <returns></returns>
	t.detach();

	/// <summary>
	/// 객체의 Thread 가 살아있는지 죽어있는지 확인하는 용도로 쓴다.
	/// </summary>
	/// <returns></returns>
	t.joinable();

	/// <summary>
	/// Thread 가 끝날때까지 기다려준다. 대기 상태
	/// </summary>
	/// <returns></returns>
	t.join();

	if (t.joinable())
		t.join();

	t2.join();
}
```

### 원자성에 대해서

```
//int sum = 0;
atomic<int32> sum = 0;

void Add() {
	for (int32 i = 0; i < 100000; ++i) {
		//sum++;
		sum.fetch_add(1);
	}
}

void Sub() {
	for (int32 i = 0; i < 100000; ++i) {
		//sum--;
		sum.fetch_add(-1);
	}
}


int main()
{
	Add();
	Sub();
	/// 절차로 진행할 경우 결과 0이 출력.
	cout << sum << endl;


	thread t1(Add);
	thread t2(Sub);
	t1.join();
	t2.join();

	/// 쓰레드로 진행한 결과 0 이 아닌 숫자가 출력
	/// 멀티 쓰레드 환경에서는 0이 아닌 이유 <- 쓰레드 공유 데이터 문제
	/// 간단한 덧셈, 뺄셈 한줄짜리 로직이라도, CPU는 여러번의 명령으로 해당 로직을 처리한다.
	/// 명령어의 원자성을 보장해야한다. atomic 연산을 해야 한다.
	cout << sum << endl;
}

/*
 명령어는 한줄에서 싱행되지 않는다.
 CPU 설계상 + 를 한번에 할수 없기 때문에 3 ~ 4줄로 나누어서 연산 처리를 하게 된다.
00007FF65E9627BB  mov         dword ptr [rbp+4],0
00007FF65E9627C2  jmp         Sub + 2Ch(07FF65E9627CCh)
00007FF65E9627C4  mov         eax, dword ptr[rbp + 4]
00007FF65E9627C7  inc         eax
00007FF65E9627C9  mov         dword ptr[rbp + 4], eax
00007FF65E9627CC  cmp         dword ptr[rbp + 4], 186A0h
00007FF65E9627D3  jge         Sub + 45h(07FF65E9627E5h)
sum--;
00007FF65E9627D5  mov         eax, dword ptr[sum(07FF65E96F440h)]
00007FF65E9627DB  dec         eax
00007FF65E9627DD  mov         dword ptr[sum(07FF65E96F440h)], eax

Sum++ 연산
	//cpu 연산 방법
	//int32 eax = sum;
	//eax = eax + 1;
	//sum = eax;
Sum -- 연산
	//cpu 연산 방법
	//int32 eax = sum;
	//eax = eax - 1;
	//sum = eax;

//atomic : All-Or-Nothing

C++에 Atomic 이 클래스 제공
Atomic 클래스를 사용하면 해당 메모리의 Atomic 을 보장한다.
Atomic연산은 Thread경합을 막아주는 연산이기 때문에, 필수로 공유 메모리의 Atomic을 보장해야 하는 상황에만 상요해야 한다.

*/
```


DeadLock에 의해서 꼬이는 버그가 발생할 빈도가 높다.
Live상황에서 인원수가 많아지면 발생해짐.
DeadLock발생 전에 해당 이슈를 방어하는게 좋다.

그래프 알고리즘을 통해서 사이클 중에 판별한다.
- 행렬을 2차원 행렬을 사용한다.
- DFS 깊이 우선 탐색
	- 순방향 간선
	- 역방향 간선
	- 교차 간선
- BFS 넓이 우선 탐색



##레퍼런스 카운팅
레퍼런스 카운팅에 개념, 다른 누군가가 해당 객체를 참조하게 되면 Ref Count를 증가시켜준다.
만약 참조를 하지 않는 상태가 된다면 RefCount를 감소 시켜준다.
->객체 초기 생성 시 RefCount = 1
->RefCount == 0 이 될 경우 객체를 소멸시킨다.

레퍼런스 카운팅은 하나의 쓰레드 환경에서는 문제없이 동작한다.
하지만 멀티 쓰레드 환경에서는 문제가 된다.

1. AddRef를 수동으로 관리하기 때문에 문제가 발생한다. [프로그래머의 부주의.] <- 수동으로 ref를 각가 객체에서 증가시켜주는 방식.
2. ref++ 코드 자체가 atommic하게 동작하지 않는다. <- atomic<> 으로 변수 바꾸면 문제 해결.
3. 실행 흐름 중간에 누군가 끼어들 수 있다. -> 멀티 쓰레드 환경에서 문제.
3-1. 예시 구문
```
class Missile : public RefCountable
{
public:
	void SetTarget(Wraight* target) {
		_target = target;
					<-------------------------------- 사용 할려고 했는데, 다른쓰레드에서 이미 Ref를 감소시켜서 날라가버리는 상태가 발생 할 수 있다.
		target->AddRef();
	}

	void Update() {

		if (_target == nullptr)
			return;

		int pox = _target->_posX;
		int poy = _target->_posY;

		if (_target->_hp == 0) {
			_target->ReleaseRef();
			_target = nullptr;
		}

		//쫓아 가간다.
	}

	Wraight* _target = nullptr;
};
```