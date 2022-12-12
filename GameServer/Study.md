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

