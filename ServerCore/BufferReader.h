#pragma once

/// <summary>
/// BufferReader
/// </summary>

//[     ][   ]포인터와 영역을 관리하는 역활을 한다.
class BufferReader
{
public:
	BufferReader();
	BufferReader(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferReader();

	BYTE*	Buffer() { return _buffer; }
	uint32	Size() { return _size; }
	uint32	ReadSize() { return _pos; }
	uint32	FreeSize() { return _size - _pos; }

	//해당 위치에서 데이터 감지.
	template<typename T>
	bool Peek(T* dest) { return Peek(dest, sizeof(T)); }
	bool Peek(void* dest, uint32 len);

	template<typename T>
	bool Read(T* dest) { return Read(dest, sizeof(T)); }
	bool Read(void* dest, uint32 len);

	template<typename T>
	BufferReader& operator>>(OUT T& dest);

private:
	BYTE* _buffer = nullptr;
	uint32	_size = 0;
	uint32	_pos = 0;
};

template<typename T>
inline BufferReader& BufferReader::operator>>(OUT T& dest) {
	//여태까지 읽은 위치에서 dest에서 꺼내서 복사해서 사용한다.
	dest = *reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);	//읽은 위치 앞으로 땡기기.
	return *this;
}

