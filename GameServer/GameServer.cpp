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


