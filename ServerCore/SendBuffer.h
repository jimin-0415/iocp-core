#pragma once

/// <summary>
/// Send Buffer
/// </summary>
/// SendBuffer 도 RefCount 대상
class SendBuffer : public enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(int32 bufferSize);
	~SendBuffer();

	BYTE* Buffer() { return _buffer.data(); }
	int32 WriteSize() { return _writeSize; }
	int32 Capacity() { return static_cast<int32>( _buffer.size()); }

	void CopyData(void* data, int32 len);

private:
	Vector<BYTE> _buffer;
	int32		_writeSize = 0; //실질적인 버퍼를 사용하는 크기, 이 크기는 write 사이즈보다 클 수 있다.
};

