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
	
	template<typename T>	//오른값 참조 -> 문제점 템플릿이 붙는 순간 보편 참조로 바뀌어버린다.
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

//여태까지 읽은 위치에서 dest에서 꺼내서 복사해서 사용한다.
template<typename T>
BufferWriter& BufferWriter::operator<<(T&& src) {
	
	//T 의 레퍼런스를 떼겠다. -> T=int 일경우 보편참조로 const int& 형태로 들어옴
	//아래 코드 호출 후 const int& -> const int 로 변경
	using DataType = std::remove_reference_t<T>;	

	//오른값 참조가 아니기 때문에 move 가 아니라 forward 로 변경
	*reinterpret_cast<DataType*>(&_buffer[_pos]) = std::forward<DataType>(src);
	_pos += sizeof(T);
	return *this;
}

//보편 참조를 하게 되면 
//왼값이면 const T& 로 변환
//오른값이면 T&& 오른값으로 남는다.
//int 를 넘기면 const int& 로 넘겨진다.