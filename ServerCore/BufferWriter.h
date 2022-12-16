#pragma once
class BufferWriter
{
public:
	BufferWriter();
	BufferWriter(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferWriter();

	BYTE*	Buffer() { return _buffer; }
	uint32	Size() { return _size; }
	uint32	WriteSize() { return _pos; }
	uint32	FreeSize() { return _size - _pos; }

	//해당 위치에서 데이터 감지.
	template<typename T>
	bool Write(T* dest) { return Write(dest, sizeof(T)); }
	bool Write(void* src, uint32 len);

	template<typename T>
	T*  Reserve();
	

	template<typename T>	//왼값 참조
	BufferWriter& operator<<(const T& src);

	template<typename T>	//오른값 참조
	BufferWriter& operator<<(T&& src);
private:
	BYTE* _buffer = nullptr;
	uint32	_size = 0;
	uint32	_pos = 0;
};

template<typename T>
T* BufferWriter::Reserve() {
	if (FreeSize() < sizeof(T))
		return nullptr;

	T* ret = reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);
	return ret;
}

template<typename T>
BufferWriter& BufferWriter::operator<<(const T& src) {
	//여태까지 읽은 위치에서 dest에서 꺼내서 복사해서 사용한다.
	*reinterpret_cast<T*>(&_buffer[_pos]) = src;
	_pos += sizeof(T);
	return *this;
}

template<typename T>
BufferWriter& BufferWriter::operator<<(T&& src) {
	//여태까지 읽은 위치에서 dest에서 꺼내서 복사해서 사용한다.
	*reinterpret_cast<T*>(&_buffer[_pos]) = std::move(src);
	_pos += sizeof(T);
	return *this;
}