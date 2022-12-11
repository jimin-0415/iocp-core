#pragma once
#include <thread>
#include <functional>

/// <summary>
///  ThreadManager
/// </summary>
class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();
					//function<input<output>>
	void	Launch(function<void(void)> callback);
	void	Join();

	static void InitTLS();
	static void DestroyTLS();

private:
	Mutex			_lock;
	vector<thread>	_threads;
};

