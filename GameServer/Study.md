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

1. AddRef를 수동으로 관리하기 때문에 문제가 발생한다. [프로그래머의 부주의.] <- 수동으로 ref를 각가 객체에서 증가시켜주는 방식.   [자동이 필요]
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
													  해당 문제는 하나의 쓰레드에서는 발생하지 않지만, 멀티 쓰레드에서는 일어날 수 있는 문제.
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

SmartPointer

직접 SharedPtr을 만들경우와 만들지 않을경우.

1. 직접 만들 경우 : 모든 객체에 최상위 객체를 만들어서 기본 상속을 내려줘야 한다.
 -> 완벽히 우리가 수정 가능하다, 
 한계.
 1. 이미 만들어진 클래스 대상으로 사용이 불가능하다. <- 꼭 상속을 받아야만 사용이 가능하다.
	-> 템플릿 함수에서 해당 인터페이스를 제공하지 않으면 문제가 된다.
	
 2. 순환(Cycle) 문제가 생길 수 있다. [기본 표준도 동일함]
  -> 

2. 외부 라이브러리를 사용할 경우 
-> 우리가 수정 불가, 상속을 받지 않음

using KnightRef = TSharedPtr<class Knight>;
using InventoryRef = TSharedPtr<class Inventory>;

class Knight : public RefCountable 
{
public:
	Knight() {
		cout << "khight()" << endl;
	}

	~Knight() {
		cout << "~ khight()" << endl;
	}

public:
	void SetTarget(KnightRef target) {
		_target = target;
	}

	void SetInventory(InventoryRef& inventory) {
		_inventory = inventory;
	}

private:
	KnightRef _target;
	InventoryRef _inventory = nullptr;
};



class Inventory : public RefCountable
{
public:
	Inventory(KnightRef knight) 
		:_target(**knight) //ref에서 역참조 
	{
	}

private:
	Knight& _target; 
};

int main()
{
	KnightRef k1(new Knight());
	k1->ReleaseRef();
	KnightRef k2(new Knight());
	k2->ReleaseRef();

	//서로 주시하고 있기 때문에 순환 참조를 하게 된다. Thread의 순환 참조와 같음
	//컴포넌트 패턴처럼, 다른 클래스를 참조하고 있을 경우 발생한다.
	k1->SetTarget(k2);
	k2->SetTarget(k1);

	//해결 1
	k1->SetTarget(nullptr);
	k2->SetTarget(nullptr);

	k1 = nullptr;
	k2 = nullptr;


	KnightRef k3(new Knight());
	k3->SetInventory(*new InventoryRef());
	/*
	* 해당 상황은 문제가 되지 않습니다.
	* class Inventory : public RefCountable
	{
		public:
			Inventory(KnightRef knight)
				:_target(**knight) //ref에서 역참조
			{
			}

		private:
			Knight& _target; <- KnightRef& _target; 으로 참조할 경우 SmartPointer 순환 참조 현상이 발생함
	};
	*/
	//순환 참조를 할 경우, 메모리가 해지 되지 않기 때문에 메모리 사용량이 늘어 메모리 Leek이 발생합니다.
	//순환 참조는 표준 SmartPointer도 동일하다. 순환 참조는..

	//1. Unqiue Ptr
	// 1.스마트 포인터끼리 복사가 불가능하다.
	// 2.복사가 불가능하기 때문에 이동을 시켜야 한다.
	// 3.객체가 자동 소멸된다.
	unique_ptr<Knight> k4 = make_unique<Knight>();
	//unique_ptr<Knight> k3 = k2; [x]
	unique_ptr<Knight> k5 = std::move(k4);


	//3. Shared Ptr
	// ref를 상속받지 않은 객체에도 사용가능하다.
	// 직접 만든 ref방식과 차이가 있다.
	// Ref Count 가 상속 관계가 아니구, has a 관계로 구성되어있다.
	//[ref][RefCountingBlock]
	shared_ptr<Knight>spr3 = make_shared<Knight>(); //한번에 만들어 버린다.
	shared_ptr<Knight> spr4 (new Knight());			//make_shared 란 생성자를 통해서 만드는 부분하고 차이가 있다.
	
	//make_shared: 미리 할당할 공간을 잡은 후 한번에 만든다.
	//(new Knight)로 생성할 경우 knight 를 할 당 후 ref 공간을 할당한다.
	//RefCOuntBlock 에서는 _Use카운트와 Week 카운트를 두개의 변수를 가지고 있다.

	shared_ptr<Knight> spr;
	shared_ptr<Knight>spr2 = spr;

	//2. Week Ptr
	// WeekPtr은 RefCountBlock을 참조해서 해당 객체가 진짜로 사라졌는지 아니면 사라지지 않았는지 알 수 있지만, 관리객체의 수명에는 관여하지 않는다.
	// 사이클 문제를 해결할 수 있다. 
	weak_ptr<Knight> wek1 = spr2;
	bool expire = wek1.expired(); //weekpointer 만료 확인.
	shared_ptr<Knight> sp2 = wek1.lock();	//lock을 통해서 shared_ptr로 변환해서 사용하는 방식.
	// 만약 해당 객체가 모두 해제되면 lock() 을 하면 null이 됨.

	//RefCountBlock 
	//	(useCount) : Shared
	//	(weekCount) : week
	// -> useCount 가 0이면 관리하는 객체는 날라간다. 하지만 RefCountBlock은 소멸되지 않는다.
	// -> weekCount가 

	//week_ptr : <- shared만 사용할 수 있고, week만 사용할 수 있다.
	// 관리 객체에 대한 수명 주기에는 영향을 주지 않는다. 
	//막상 week_ptr을 사용하면, 두번에 걸쳐서 작업을 해야되서 번거롭다. <- 프로젝트에 따라서 선택의 영역이다.
	
	return 0;
}



##메모리 관리

C# 은 메모리 관리 다 해줌
C++은 메모리를 커스터마이징이 가능하다.
메모리 동적 할당 해제를 직접 할 수 있다.

오브젝트, 메모리 풀링으로 한다.

커널쪽에 메모리를 관리하긴하지만 그래도 context switching이 이루어 질 수 있다.

1. 메모리를 큰 메모리 영역으로 할당을 해서 쪼개면, 굳이 운영체제 커널에 메모리 할당을 요청할 필요없다.
2. 주기적으로 메모리 할당 해제를 하다보면, 메모리 파편화 문제가 생길 수 있습니다.
요즘은 마소에서 잘 해놔서 굳이 풀링으로 할 필요도 없을거라고 함. 하지만 ..그래도 필요.

//new operator overloading (Global)
void* operator new(size_t size) {
	cout << "new ! " << size << endl;
	void* ptr = ::malloc(size);
	return ptr;
}

void operator delete(void* ptr) {
	cout << "delete ! " << endl;
	::free(ptr);
}

void* operator new[](size_t size) {
	cout << "new []" << size << endl;
	void* ptr = ::malloc(size);
	return ptr;
}


void operator delete[](void* ptr) {
	cout << "delete []" << endl;
	::free(ptr);
}

모든 애들한테 할당 관련되서 오버로딩이 됨.
new delete 는 오버로딩 함수이다.

//xnew , xdelete 사용자가 원하는 정책으로 할당 delete 를 할 수 있다.
//placement new 를 통해서 할당된 메모리의 생성자 호출
//인자가 여러개 있을 수 있다. -> 배러릭 템플릿으로 가변적 인자를 받을 수 있게 변경됬다.

/*
//옛날에는 함수가 여러 인자를 받을 수 있게 이렇게 template을 만들었다.
template<typename Type>
Type* xnew() {
	Type* memory = static_cast<Type*>(BaseAllocator::Alloc(sizeof(Type)));

	//placement new
	new(memory)Type(args);
	return memory;
}

template<typename Type, typename T1>
Type* xnew(T1 t1) {
	Type* memory = static_cast<Type*>(BaseAllocator::Alloc(sizeof(Type)));

	//placement new
	new(memory)Type(t1);
	return memory;
}

template<typename Type, typename T1, typename T2>
Type* xnew(T1 t1, T2 t2) {
	Type* memory = static_cast<Type*>(BaseAllocator::Alloc(sizeof(Type)));

	//placement new
	new(memory)Type(t1, t2);
	return memory;
}

##메모리 오염 문제
메모리 오염일 경우 바로 Crash가 나면 좋지만... Crash가 나지 않으면 2주 뒤에 발생 할 수 있음.
나 혼자만 짜는게 아니고.. 여러 작업자들이 작업하기 때문에.. 추적하기 어려움. [예측범위에서 벗어남]

Knight* k1 = new Knight();
	k1->_hp = 200;
	k1->_mp = 300;
	delete k1;
	k1 = nullptr;
	k1->_hp = 100; //k1 사용 가능 //Use-After-Free
	//메모리 삭제를 하더라도, 메모리 접그 후 사용이 가능합니다.
	//스마트 포인터로 어느정도 커버가 가능함

	//문제는 해제한 메모리에 대한 접근을 할경우, 대부분 공유 메모리에서 해당 문제가 많이 발생합니다.
	vector<int32> v{ 1 ,2, 3, 4, 5 };
	for (int32 i = 0; i < 5; ++i) {
		int32 value = v[i]; 

		//TODO 
		if (value = 3) {
			v.clear();
		}
	}

	//Casting 문제 메모리 오염
	//메모리 오버플로우현상 발생, 강제 static_cast 를 할경우
	//대부분 dynamic_cast 는 성능상 안좋기 떄문에 확실한 경우 static_cast 를 사용합니다.

	Player* p = new Player();
	Knight* k = static_cast<Knight*>(p);

	k->_hp = 200;
	
OS 레벨에서 Page 단위만큼 보안 레벨을 설정 할 수 있다. R or W or RW or x

SYSTEM_INFO info;	//system 정보를 가져온다.
	::GetSystemInfo(&info);

	info.dwPageSize; //4kb page 사이즈를 가져온다.
	info.dwAllocationGranularity;		//64kb

				//아무 위치에 ,4 byte 공간을 , 예약|커밋을 RW 권한으로 해줘
	int* test = (int*)::VirtualAlloc(NULL, 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	*test = 100;
	::VirtualFree(test, 0, MEM_RELEASE);

	*test = 200; //Crash가 발생 <- new delete 예약어와 다른점.

	//VirtualAlloc, Free 를 사용할 경우 운영체제 커널단에 메모리 예약과 해제를 하기 떄문에, 만약 해제된 메모리에 접근할 경우 Crash가 발생함.
	//하지만 C++ 에서 제공해주는 new , delete 예약어를 사용할 경우 CheckCrash 까지 발생하지 않음을 유추할 수 있음. 
	



## STLAllocator 
-> STL의 경우 기본 new , delete 를 통해서 메모리 할당을 하게 된다.
-> 이럴 경우 우리가 만든 Allocator 를 사용하지 못한다.
-> 이럴때 STLAllocator 를 할당해서 사용할 수 있도록 한다.
STL Allocator 를 사용할 경우 기본 Allocator 가아닌 내가 직접 커스마이징한 Allocatpr 를 사용할 수 있다.
메모리 할당, 해제 정책을 사용할 수 있다.
Allocator가 있다고 기본 new , delete 를 사용하지 못하는것은 아니다. 다만 내가 직접 해당 메모리 관리를 할 수 없다는 단점만 있을 뿐이다.

## 메모리 풀링
- 해제 할당 시 context 스위칭이 자주 발생함.
- 메모리 파편화가 발생할 수 있다. 하지만 이건 OS에서 이런거 잘 처리해 줘서 메모리 파편화 까지 신경쓰지 않아도 될거 같다.
- 메모리 풀링은 선택의 사항이 되었다.

- 메모리를 다시 사용하기 위해서 메모리 풀링을 사용합니다.
-> 메모리를 해제하게 되면 systemCall 호출, 할당도 system call 호출
- 풀링 생각할 점 : 메모리 사이즈는 고정이 아니다. [ObjectPool과 다름]

[32] [64] [128] [265] [] [] []
[                      ]
메모리 풀 정책 -> 다양한 크기의 풀을 만든다.

동일한 크기끼리 관리할것이냐?
아니면 다른 크기끼리 관리할 것이냐?

//[Memory Header][Data]
//실제 메모리를 보면 Write객체 할당할때 여러 정보를 같이 넣어준다.

//MemoryPool : 동일 크기의 메모리 풀
-> Pool에 해당 메모리 Chunk가 있으면 가져온다.
-> Pool에 메모라가 없으면 새로 할당한다 
	-> 4096 크기 4kbyte를 넘어가면 기본 malloc으로 할당한다.
	-> 이하일 경우 해당 PoolSize에 맞는 메모리를 할당 받는다.



MemoryPool 아쉬운점
1. 메모리풀에서 Pop(), Push()할경우 WriteLock을 통해서 Thread가 경합을 해야한다는점.
2. 내부적 방식이 동적 배열로 되어있다.


##LockFreeStack
### 1. SingleTread LockFreeStack
/// <summary>
/// //SingleThread LockFreeStack
/// </summary>

struct SListEntry 
{
	SListEntry* next;
};

class Data // : public SListEntry
{

public:
	SListEntry _entry;

	int data1;
	int data2;
};

struct SListHeader 
{
	SListEntry* next = nullptr;
};


void InitializeHeader(SListHeader* header);
void PushEntryList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntryList(SListHeader* header);

void InitializeHeader(SListHeader* header) {
	header->next = nullptr;
}

//header [ nullptr <-entry ] 
void PushEntryList(SListHeader* header, SListEntry* entry) {
	entry->next = header->next;
	header->next = entry;
}

SListEntry* PopEntryList(SListHeader* header) {
	SListEntry* first = header->next;

	if (first != nullptr) {
		header->next = first->next;
	}
	return first;
}

###2. MultiThread Lock Free Stack

///MultiThread LockFreeStack
///문제 1. 꺼내온 녀석의 메모리가 이미 제거되어 Crash 발생 가능성이 있다.
/// -> node를 직접 관리하지 않기 때문에.
/// 2. ABA 프로블럼이 발생할 수 있다.
/// 
void InitializeHeader(SListHeader* header) {
	header->next = nullptr;
}

//header [ nullptr <-entry ] 
void PushEntryList(SListHeader* header, SListEntry* entry) {
	entry->next = header->next;
	//entry, entry->next 비교
	while (::InterlockedCompareExchange64((int64*)&header->next, (int64) entry, (int64)entry->next) == 0) {
															//header->next = entry;
	}

}


//1. 이렇게만 꺼내오게되면 꺼내온 녀석이 어디선가 delete가 되면 Crash 가 발생할 수 있다.
//->그렇기 때문에 해당 Node에 refCount를 주어서 소멸되지 않게 해야한다.
//2. ABA Problem 현상이 발생 할 수 있다. -> 메모리 주소 하나만을 가지고 비교할경우 interlocked 써도 해당 문제는 발생함
// 만약 Header가 5000이라면, Header에다가 6000을 넣는다.
//-> [5000] -> [6000] -> [7000]
//	 [Header]
// 만약 Header가 5000이라면, Header에다가 6000을 넣는다. 이코드 실행할려는 찰나 다른 쓰레드에서 Pop, Pop해서 두개다 가져가면
// 가져갔다가 공고롭게 같은 메모리 주소를 다시 인서트 하게된면?
//-> [5000] -> [7000] 이상태가 되었을때. -> 5000꺼내고 -> 6000을 넣어벌니다. 
//-> 기대값 [header 7000] -> 결과값 [header 6000]

SListEntry* PopEntryList(SListHeader* header) {
	SListEntry* expected = header->next;

	while (expected
		&& ::InterlockedCompareExchange64((int64*)&header->next, (int64)expected->next, (int64)expected) == 0) {
		/*if (first != nullptr) {
			header->next = first->next;
		}*/

	}
	return expected;
}

###3. ABA 해결 Lock Free Stack
#include "pch.h"
#include "LockFreeStack.h"

#pragma once

/// <summary>
/// //SingleThread LockFreeStack
/// </summary>

//16byte로 정렬
DECLSPEC_ALIGN(16)
struct SListEntry 
{
	SListEntry* next;
};

DECLSPEC_ALIGN(16)
class Data // : public SListEntry
{

public:
	SListEntry _entry;
	int64 _rand = rand() % 1000;
};

DECLSPEC_ALIGN(16)
struct SListHeader 
{
	SListHeader() {
		alignment = 0;
		region = 0;
	}
	union 
	{
		struct
		{
			uint64 alignment;
			uint64 region;
		} DUMMYSTRUCTNAME;

		struct
		{
			uint64 depth : 16;		//data 가 들어간 개수에 따라 1씩 증가
			uint64 sequence : 48;	//
			uint64 reserved : 4;
			uint64 next : 60;
		} HeaderX64;
	};
};


void InitializeHeader(SListHeader* header);
void PushEntryList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntryList(SListHeader* header);


void InitializeHeader(SListHeader* header) {
	header->alignment = 0;
	header->region = 0;
}


void PushEntryList(SListHeader* header, SListEntry* entry) {
	SListHeader expected = {};	//예상
	SListHeader desired = {};	//원하는
	
	//16byte 정렬은 하위 4bit는 0000이다. 
	desired.HeaderX64.next = (((uint64)entry) >> 4); //60bit에 저장, 16bit로 정렬되었기 때문에 쉬프트연산으로 4비트 땡김.
														//-> 땡기는 이유 앞 4bit 공간을 다른 목적으로 사용 가능
	while (true) {
		expected = *header;

		//이 사이에 변경 가능하다. <- 다른쓰레드가
		entry->next = (SListEntry*)(((uint64)expected.HeaderX64.next) << 4); //header 복원
		desired.HeaderX64.depth = expected.HeaderX64.depth + 1;		 
		desired.HeaderX64.sequence = expected.HeaderX64.sequence + 1; 

		if (::InterlockedCompareExchange128((int64*)header, desired.region, desired.alignment, (int64*)&expected) == 1)
			break;
	}
}

SListEntry* PopEntryList(SListHeader* header) {
	SListHeader expected = {};	//예상
	SListHeader desired = {};	//원하는
	SListEntry* entry = nullptr;
	
	while (true) {
		expected = *header;

		entry = (SListEntry*)(((uint64)expected.HeaderX64.next) << 4);
		if (entry == nullptr)
			break;

		//Use-After-Free 문제는 여전히 존재.
		desired.HeaderX64.next = ((uint64)entry->next) >> 4;
		desired.HeaderX64.depth = expected.HeaderX64.depth - 1;
		desired.HeaderX64.sequence = expected.HeaderX64.sequence + 1;

		if (::InterlockedCompareExchange128((int64*)header, desired.region, desired.alignment, (int64*)&expected) == 1)
			break;
	}
	return entry;
}

//해당 코드 ABA 문제점은 해결되었다.
//하지만 Use-After-Free문제는 여전히 존재합니다.
//LockFreeStack은 만들어 쓰는것보다 MS에서 제공해주는것 사용하기.
//MS에서도 16byte 정렬을 맞추라고 하는데, 이 구현 방법처럼 작동하기 때문이다.
//하위 4bit는 0000으로 확신이 가능하기 때문에.


###ObjectPool
메모리 풀의 축소 개념 
특정 오브젝트를 재사용한다.
-> 만약 특정 메모리의 오염이 일어날 경우 다른 곳에서 해당 문제가 발생할 경우가 생긴다.
그럴 경우 오브젝트 풀을 쓰게 되면 명확히 어떤 클래스에서 메모리오염이 생겼는지 알 수 있다.

그리고 메모리 풀과, StompMemoryPool을 모두 사용 불가하기 때문에, 케이스에 따라서 별도로 사용할 수 있도로 한다.

###TypeCast


### Socket 
#### Socket Server
```
#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main()
{
    //Winsock 라이브러리 초기화 ws2_32 초기화
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        int errCode = ::WSAGetLastError();
        cout << "Socket ErrorCode :" << errCode << endl;
        return 0;
    }

    SOCKADDR_IN serverAddr; //ip4 일경우 이 구조체 사용
    ::memset(&serverAddr, 0, sizeof(serverAddr)); //메모리 초기화
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);   //모든 주소 연결 
    serverAddr.sin_port = ::htons(7777);    //http : 80 같은 번호는 예약이 되어 있다.

    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        int errCode = ::WSAGetLastError();
        cout << "Bind ErrorCode :" << errCode << endl;
        return 0;
    }

    //일단 대기열 10개
    if (::listen(listenSocket, 10) == SOCKET_ERROR) {
        int errCode = ::WSAGetLastError();
        cout << "Listen ErrorCode :" << errCode << endl;
        return 0;
    }

    while (true) {
        SOCKADDR_IN clientAddr;
        ::memset(&clientAddr, 0, sizeof(clientAddr)); //메모리 초기화
        int32 addresslen = sizeof(clientAddr);

        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addresslen);
        if (clientSocket == INVALID_SOCKET) {
            int errCode = ::WSAGetLastError();
            cout << "Socket Accept ErrorCode :" << errCode << endl;
            return 0;
        }

        char ipAddress[16];
        ::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
        cout << "Client Connected IP = " << ipAddress << endl;
    }

    ::WSACleanup();
	return 0;
}

```

#### Socket Client
```

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main()
{
    //Winsock 라이브러리 초기화 ws2_32 초기화
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    //af : address Familiy 주소 체계 
    //type : Tcp, Udp
    //protocol : 0
    // return : descriptor <- 정수 받은 각 socket 의 식별 번호
    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        int errCode = ::WSAGetLastError();
        cout << "Socket ErrorCode :" << errCode << endl;
        return 0;
    }

    SOCKADDR_IN serverAddr; //ip4 일경우 이 구조체 사용
    ::memset(&serverAddr, 0, sizeof(serverAddr)); //메모리 초기화
    serverAddr.sin_family = AF_INET;
    //serverAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1") //너무 옛날 버전
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = ::htons(7777);    //http : 80 같은 번호는 예약이 되어 있다.

    //host to network short
    //Little-Endian vs Big-Endian
    //ex) 0x12345678    4바이트 정수 저장
    // low [0x78][0x56][0x34][0x12] high <   little , cpu에 따라서 다르다, 요즘은 모두 little 엔디안 방식 [client] 
    // <- 로컬 컴퓨터 환경은 little 방식이다
     // low [0x12][0x34][0x56][0x78] high <   Big -> 받는 쪽은 반대로 받아야 한다. bigEndian 방식으로. [server]
    
    if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        int errCode = ::WSAGetLastError();
        cout << "Connet ErrorCode :" << errCode << endl;
        return 0;
    }

    //Connect Success
    cout << "Connected To Server!" << endl;

    while (true) {

        this_thread::sleep_for(1s);
    }

    //소켓 리소스 반환
    ::closesocket(clientSocket);

    //윈속 종료
    ::WSACleanup();
}

```

##Socket 

소켓을 받지도 않았는데, success 가 떨어지는 원인. 

1. 소켓이 성공적으로 만들어진 상태
소켓이 만들어지면, 
운영체제에서 관리하는 커널영역의 recv , send buffer 가 존재한다.

1. 클라이언트에서 send 함수를 호출한다.
-> 커널 영역의 SendBuffer에 보낼 데이터가 정상적으로 복사되었다. <- success
-> 커널 영역의 SendBuffer가 꽉찰 경우 Blocking 처리가된다. Context Swtiching에 의해 해당 process는 대기 상태로 전환된다.
-> 커널 영역의 SendBuffer가 꽉차고 Server의 Recv 버퍼가 꽉찰 경우 : Server의 Recv 버퍼 공간이 생기기 전까지 블로킹 상태가 된다.

2. Sever의 recv 함수는 커널의 Recv Buffer로 부터 데이터를 복사해오는 역확을 한다. <- success
-> 만약 커널 영역의 Recv Buffer가 빈상태에서 recv 함수를 호출하게 되면 Blocking 상태로 전한된다.
 
블로킹 형태의 소켓 함수를 사용하면 동시처리를 할 수 없다. -> 나중에는 논 블로킹 함수로 변경이 필요.
현재는 흐름만 알기.

Send는 빠른데, Recv가 느린경우.
1. 커널 버퍼는 경계선이 없다.
2. 한번에 100바이트를 10번 보냈을 경우 recv 는 어떻게 동작하냐.
3. 한번 에 1000바이트씩 받아버린다.

TCP는 바운더리에 대한 개념이 없다.

2. 만약 recv에 남은 공간에 10byte인데 client에서 sendBuffer가 100 바이트를 보낸다면 10byte만 보내지고 
나머지 부분은 recvBuffer의 공간이 남게 되면 전송하게 된다.

### TCP UDP 차이점

어플리케이션 
트랜스포트 -> TCP/UDP는 해당 정책이다.
네트워크
데이터링크
피지컬

TCP vs UDP 차이점은
Tcp
연결형 서비스
-> 연결을 위해 할당되는 논리적 경로가 있다.
-> 전송 순서가 보장된다.

속도 신뢰성
-> 분실 일어나면 다시 전송
-> 물건을 주고 받을 상황이 아니면 일부만 보냄 (흐름/ 혼잡제어) -> 상대방에 입장을 고려해서 데이터를 보냄
-> 여러 정책적으로 고려핡게 많아 속도가 나쁘다.

데이터 BounDary
-> 데이터의 경계 개념이 없다. -> 컨베이너 벨트처럼 간다. -> 순서는 보장되지만 데이터 경계가 없다.
데이터가 어떻게 분리될지 모든다. [Hello][World] -> [Hel][lowWorld] 이렇게 받을 수 있다.

UDP
-> 연결이라는 개념이 없다.
-> 전송 순서가 보장되지 않는다.
-> 경계(Boundary)의 개념이 있다. [Good]

속도 신뢰성
-> 분실에 대한 책임 없음
-> 보내고 생각한다.
-> 단순해서 속도가 빠르다

데이터 BounDary
-> 데이터 경계 개념이 있다.
대신 순서를 보장하지 않는다. [Hello][World] -> [World][Hello] 경계의 개념이 있다.


속도를 살리면서 UDP나쁜 정책을 보완함 Reliable UDP

UDP 동작 방식 - 만약 SendBuffer, Recv Buffer 꽉찰 경우
-> Client에서 막 SendBuffer를 보냄 <- success - 분실의 책임이 없기 때문
-> Server Recv 에서는 받지 못함 [꽉차서] <- 패킷 유실로 이루어짐

//UDP
-> port번호는 send 하는 시점에 결정됨. 
-> 100byte 10개를 한번에 보냈을 경우 바운더리가 있기때문에 .100byte 씩 읽는다.

//TCP
-> port 번호는 connect 하는 시점에 결정됨. 

### Win소켓 옵션 설정
//Winsock 라이브러리 초기화 ws2_32 초기화
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        HandleError("Socket");
        return 0;
    }

    SOCKADDR_IN serverAddr; //ip4 일경우 이 구조체 사용
    ::memset(&serverAddr, 0, sizeof(serverAddr)); //메모리 초기화
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);   //모든 주소 연결 
    serverAddr.sin_port = ::htons(7777);    //http : 80 같은 번호는 예약이 되어 있다.

    //커널의 RECV, SEND 버퍼 크기도 수정이 가능하다.
    //옵션을 해석하고 처리할 주체? [level]
    //소켓 코드 -> SOL_SOKET
    // IPv4 -> IPPROTO_IP
    // TCP 프로토콜 -> IPPROTO_TCP

    // SO_KEEPALIVE //주기적으로 연결 상태 여부 확인 (TCP Only)
    // 상대방이 소리 소문 없이 연결을 끊는경우가 있기 때문에..
    // 주기 적으로 TCP 프로토콜 연결 상태 확인 -> 끊어진 연결 감지.

    bool enable = true;
    ::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));

    // SO_LINGER = 지연하다.
    // 송신 버퍼에 있는 데이터를 보낼것인가? 날릴것인가?
    // Send -> closesocket 을 하면 Send 에서 보낸 버퍼에 남은 데이터는 어떻게 할것이냐
    
    // onoff =0 이면 closesocket()이 바로 리턴, 아니면 linger 초만큼 대기 (default 0)
    LINGER linger;
    linger.l_onoff = 1; //활성화
    linger.l_linger = 5; //s 단위 5초
    ::setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

    // Half - Close
    // SD_SEND : send만 막는다.
    // SD_RECEIVE : recv 막는다.
    // SD_BOTH : 둘다 막는다.
    // shutdown  전에 해당 소켓설정을 한다.
    ::shutdown(serverSocket, SD_SEND);  

    //송 수신 버퍼 크기를 4gb 그렇게 크게 못한다.
    // 문서를 보고 사용할 애들값을 세팅해 준다.
    // SO_SNDBUF = 송신버퍼 크기
    // SO_RCVBUF = 수신버퍼 크기
    int32 sendBufferSize;
    int32 optionLen = sizeof(sendBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &optionLen);
    cout << "송신 버퍼 크기 : " << sendBufferSize << endl;

    int32 recvBufferSize;
    optionLen = sizeof(recvBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&recvBufferSize, &optionLen);
    cout << "송신 버퍼 크기 : " << recvBufferSize << endl;

    // SO_REUSEADDR
    // IP 주소 및 Port를 재사용 한다.
    // 다른 프로그램 사용 혹은 서버를 강제종료하고 다시 켰을 경우 해당 찌끄레기가 남아서 해당 IP Port 바인딩 안되는 상황이 생김.
    // 이때 해당 옵션을 통해서 IP Port를 재사용한다.
    {
        //개발 단계에서 편하라고 사용함.
        bool enable = true;
        ::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));
    }

    // IPPROTO_TCP
    // TCP_NODELAY = Nagle 네이글 알고리즘 작동 여부
    // 데이터가 충분히 크면 보내고, 그렇지 않으면 데이터가 충분히 쌓일때까지 대기 !
    // 장점 : 작은 패킷이 불필요하게 많이 생성되는 일을 방지한다.
    // 단점 : 반응 시간을 손해.
    // 게임쪽에서는 거의 사용 안함. <- 직접 관리. 반응성 문제.
    {
        bool enable = true; // nagle 작동 안함.
        ::setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
    }


###블로킹 소켓
accept -> 접속한 클라가 있을때 - 블로킹 해제
connect -> 서버 접속 성공했을때 - 블로킹 해제
send, recv 요청 데이터를 송신 버퍼에 복사했을때 - 블로킹 해제
recv, recvfrom 수신 버퍼에 도착한 데이터가 있고, 이를 유저 레벨 버퍼에 복사했을 때 - 블로킹 해제

### 논블로킹 (Non-Blocking)
블로킹을 논 블로킹으로 바꾼다고 해서, 성능 이점이 없다.
현재 코드를 보면 계속적으로 Connection, Send, Recv가 오는지 확인하기 위해서 while문으로 루프를 보고있다.
불필요하게 체크를 하기 때문에 CPU를 계속 점유한 상태가 된다.
1. 소켓은 옵션을 바꾸면 논블로킹으로 바뀌는구나.
2. 논블로킹으로 바꾼다고 해서 모든 문제가 해결되는것은 아니다. - CPU 사이클 낭비를 줄여야 한다. 여러 소켓 모델로 해결


### 소켓입출력모델 - Select모델 
SELECT 모델 = (select 함수가 핵심이 된다)
    Socekt 함수 호출이 성공할 시점을 미리 알 수 있다. - blocking, Non-Blocking 모두 적용 가능.
    //문제 상황 > <- 수신 버퍼에 데이터가 없을때, read한다는 상황.
    //문제상황 2 > <- 송신버퍼가 꽉 찼을때 write한다거나 하는 상황.

    //- 블로킹 소켓 : 조건이 만족되지 않아서 블로킹 되는 상황 방지.
    //- 논블로킹 소켓 : 조건이 만족되지 않아서 불필요하게 반복체크하는 상황 예방.

    //socket set
    //1. 읽기[] 쓰기[] 예외 [] 관찰 대상을 등록한다. 모두 넣을 필요 없음
    //원하는 관찰 대상을 넣는다. 
    //예외 ? OutOfBand - Send() 마지막 인자 MSG_OOB로 특수 데이터를 보냄
    //받는 쪽에서도 recv OOB 세팅을 해야 읽을 수 있다.
    //2. select(readSet, writeSet, exceptSet); 을 넣어서 관찰 시작.
    //3. 적어도 하나의 소켓이 준비되면 리턴 <- 낙오자는 알아서 제거됨.
    // ex > 1,2,3 소켓을 대상으로 일기를 하는데.. 이것을 socekt set에 등록해서 체크를 한다.
    // 만약여기서 select를 호출하면 1.2.3을 관찰하다가 만약 2번에 데이터가 와서 수신버퍼가 왔다면 개수를 반환해 준다.
    // 그러면 읽기에는 2만 남고 나머지 1,3는 제거된다. -> 처리를 끝나면 다시 1,2,3을 읽기 select에 세팅한다.

    //함수 호출 을 체크하고 한다.
    ///장점 : 구현이 간단하고, 리소스 낭비를 줄일 수 있다.
    //담점 : 반복적으로 등록에 문제가 생김, FD_SETSIZE <- 64개까지밖에 등록을 못한다. fd_set 하나에...만약 640명이면 640개를 만들면된다.

    //fd_set read;
    // FD_ZERO : 비운다.
    //FD_ZERO(set);

    //FD_SET : 소켓 s를 넣는다.
    //FD_SET(s, &set);
    // 
    //FD_CLR : 소켓 s 를 제거
    //FD_CLR(s, &set);
    
    //FD_ISSET : 소켓 s가 set에 들어가 있으면 0이 아닌값을 리턴한다.
	select 함수는 동기 함수이다.

### 소켓 입출력 모델 - WSAEventSelect <- windows에서만 있다.


//IOCP 에 IocpObject에 데이터 오염이 발생할 수 있다.
-> IocpObject 를 refcounting 한다. -> 그렇게 되면, 이중 shared되게 되면 문제가 생긴다.
iocp Event에 나를 걸어준 객체를 알고있다.


## SendBuffer 문제

현재는 Echo 서버를 만들어서 보내고 있고 Event가 Send를 물고있게 했다.
지속적으로 복사 비용이 든다. 1명에게 Send를 보낼때도 있겠지만, 주변 사람들에게 특정 정보를 모두 보낼 경우생김
ex -> 몬스터 죽음 -> 주위 100명에게 보냄.

SendEvent* sendEvent = xnew<SendEvent>();
sendEvent->buffer.resize(len);
::memcpy(sendEvent->buffer.data(), buffer, len); //복사 비용 100번 발생

수신버퍼와 송신버퍼의 차이점에 대해서 알고있어야 한다.
선택하는 정책이 다름.

##BufferHelpers 를 통해서 Packet 관리를 진행한다.
후에 GoogleProtoBuf 사용한다.

현재는 Buff를 가져와서 직접 세팅해준다. 그렇게 하면 실수할 여지가 많다.
