#pragma once
#include <stack>
//Thread Id 만들기
extern thread_local uint32 LThreadId;
extern thread_local std::stack<int32> LLockStack;		//Lock을 했는지 안했는지 추적을 위해서 <- 추후 개별 TSL 메모리로 옮겨야 합니다
extern thread_local SendBufferChunkRef LSendBufferChunk;