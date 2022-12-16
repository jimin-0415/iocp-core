#pragma once

///
///RecvBuffer
/// 
class RecvBuffer
{
	//실제 요청 버퍼보다 크기를 더 크게 잡아준다.<- 복사 비용을 최소화 하기 위해서
	enum {
		BUFFER_COUNT = 10,
	};

public:
	RecvBuffer(int32 bufferSize);
	~RecvBuffer();

	void Clean();
	bool OnRead(int32 numOfBytes);
	bool OnWrite(int32 numOfBytes);

	BYTE* ReadPos() { return &_buffer[_readPos]; }
	BYTE* WritePos() { return &_buffer[_writePos]; }
	int32 DataSize() { return _writePos - _readPos; }
	int32 FreeSize() { return _capacity - _writePos; }
	
private:
	int32		_capacity = 0; //실제 버퍼 크기

	int32	_bufferSize = 0;
	int32	_readPos = 0;	//Read를 해야하는 Cursor의 위치 
	int32	_writePos = 0;	//Write를 해야하는 Cursor의 위치
	Vector<BYTE> _buffer;	
};

//대부분 Recv 버퍼는 Circuler 버퍼를 사용한다.대부분.
	//[R][][][] <- 유효 데이터.	-> 
	//[][R][][][][W][][][][][][]
	//[][R][][][][W][][][][][W][]

//[rw]가 만약에 만났다 : <- 더이상 읽을 데이터가 없다
// [][][][]]][][][][r][][w] <- 맨 끝 지점으로 갔다.
// [r][][w][][][][][][][][][]  -> 처음으로 복사해서 읽기