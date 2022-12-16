#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, int32 allocSize)
	:_owner(owner), _buffer(buffer), _allocSize(allocSize)
{
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::Close(uint32 writeSize)
{
	ASSERT_CRASH(_allocSize >= writeSize);
	_writeSize = writeSize;
	_owner->Close(writeSize);
}

//Send버퍼에서 일부를 사용하겠다. [ 열어줌 ] <- 사용할 만큼 사용한 후에 close
SendBufferRef SendBufferManager::Open(uint32 size)
{
	if (LSendBufferChunk == nullptr) {//결합가능
		LSendBufferChunk = Pop(); //Pop은 하나씩 꺼냄.
		LSendBufferChunk->Reset();
	}
	ASSERT_CRASH(LSendBufferChunk->IsOpen() == false);	//Open확인

	//다 쓰면 버리고 새거로 교체.
	if (LSendBufferChunk->FreeSize() < size) { //할당할려는 사이즈보다 작을경우
		LSendBufferChunk = Pop();	//Write Lock 
		LSendBufferChunk->Reset();
	}

	cout << "Free : " << LSendBufferChunk->FreeSize() << endl;

	return LSendBufferChunk->Open(size);
}

//Pull에 반납
SendBufferChunkRef SendBufferManager::Pop()
{
	cout << "Push Global SendBUffPop" << endl;
	{
		WRITE_LOCK;
		if (_sendBufferChunks.empty() == false) {
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}

	//Ref가 0이되면 메모리를 날리는게 아니라 PushGlobal 로 가서 SendBufferChunk에 넣는다.
	return SendBufferChunkRef(xnew<SendBufferChunk>(), PushGlobal);	
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	cout << "Push Global SendBUffChunck" <<endl;
	//반납한것 계속 재사용.
	GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
}

SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
	_open = false;
	_usedSize = 0;
}

void SendBufferChunk::Reset()
{
}

//어디부터 사용하냐
SendBufferRef SendBufferChunk::Open(uint32 allocSize)
{
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE); //chunk사이즈보다 큰지 확인
	ASSERT_CRASH(_open == false); //이중 오픈 확인
	
	if (allocSize > FreeSize())
		return nullptr;

	_open = true;
	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), allocSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_CRASH(_open == true);	//Open 확인
	_open = false;
	_usedSize += writeSize;
}
