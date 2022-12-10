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
