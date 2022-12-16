#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(int32 bufferSize)
	:_bufferSize(bufferSize)
{
	//Buffer를 10배 크게 잡는다.
	_capacity = bufferSize * BUFFER_COUNT;	
	_buffer.resize(_capacity);
}

RecvBuffer::~RecvBuffer()
{
}

void RecvBuffer::Clean()
{
	int32 dataSize = DataSize();
	//대부분 RW가 같은 싸이즈에 위치해서 초기화 가능하다.
	if (dataSize == 0) {	
		//읽기 + 쓰기 커서가 동일한 위치에 있을경우 리셋.
		_readPos = 0;
		_writePos = 0;
	}
	else {
		//여유 공간이 버퍼 1개 미만이면, 데이터를 앞으로 당긴다.
		if (FreeSize() < _bufferSize) {
			//복사 하기.
			::memcpy(&_buffer[0], &_buffer[_readPos], dataSize);
			_readPos = 0;
			_writePos = dataSize;
		}	
	}
}

bool RecvBuffer::OnRead(int32 numOfBytes)
{
	if (numOfBytes > DataSize())
		return false;

	_readPos += numOfBytes;
	return true;
}

bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	if (numOfBytes > FreeSize())
		return false;

	_writePos += numOfBytes;
	return true;
}
