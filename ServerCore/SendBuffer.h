#pragma once

class SendBufferChunk;
/// <summary>
/// Send Buffer
/// </summary>
/// SendBuffer 도 RefCount 대상
class SendBuffer : public enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(SendBufferChunkRef owner, BYTE* buffer, int32 allocSize);
	~SendBuffer();

	BYTE* Buffer() { return _buffer; }
	int32 WriteSize() { return _writeSize; }
	void Close(uint32 writeSize);

private:
	BYTE* _buffer;
	uint32 _allocSize = 0;
	uint32 _writeSize = 0;	//사용중인 공간
	SendBufferChunkRef	_owner; //오너가 누구인지 들고있어서 ref count를 유지한다.
};

/// <summary>
/// Send BufferChunk
/// 가장 큰 덩어리를 할당해서 메모리를 짤라 쓰겠다.
/// [][          ][    ]
/// TLS 영역별로 별도의 SendBufferChunk를 가지기 때문에 MultiThread 고려 x
/// 큰 Chunk를 잡고 짤라 쓰겠다.
/// </summary>	
class SendBufferChunk : public enable_shared_from_this<SendBufferChunk> 
{
	enum {
		SEND_BUFFER_CHUNK_SIZE = 10000,
	};

public:
	SendBufferChunk();
	~SendBufferChunk();

	void Reset();
	SendBufferRef Open(uint32 allocSize);	//열어줄 공간 size
	void Close(uint32 writeSize);	//닫아준 공간 size

	bool IsOpen() { return _open; }	//사용중인지.
	BYTE* Buffer() { return &_buffer[_usedSize]; }
	uint32 FreeSize() { return static_cast<uint32>(_buffer.size()) - _usedSize; }
	
private:
	Array<BYTE, SEND_BUFFER_CHUNK_SIZE> _buffer = { };
	bool _open = false;
	uint32 _usedSize = 0;
};



/// <summary>
/// Send BufferManager
/// </summary>
class SendBufferManager 
{
public:
	SendBufferRef	Open(uint32 size);

private:
	SendBufferChunkRef	Pop();
	void	Push(SendBufferChunkRef buffer);

	static void PushGlobal(SendBufferChunk* buffer);

private:
	USE_LOCK;
	Vector<SendBufferChunkRef> _sendBufferChunks;
};