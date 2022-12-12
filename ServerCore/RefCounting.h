#pragma once


//최상위 클래스에 RefCountable 을 상속받아서 RefCount를 상속받아 사용한다.

class RefCountable 
{
public:
	RefCountable()
		:_refCount(1){

	}
	virtual ~RefCountable() {

	}
	
	int32 GetRefCount() {
		return _refCount;
	}

	int32 AddRef() {
		return ++_refCount;
	}

	int32 ReleaseRef() {
		int32 refCount = --_refCount;
		if (refCount == 0) {
			delete this;
		}
		return refCount;
	}

protected:
	int32 _refCount;
};

template<typename T>
class TSharedPtr
{
public:
	TSharedPtr() { }
	TSharedPtr(T* ptr) {
		Set(ptr);
	}
	//copy
	TSharedPtr(const TSharedPtr& rhs) {
		Set(rhs._ptr);
	}
	//move
	TSharedPtr(TSharedPtr&& rhs) {
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
	}
	//Inheritance copy
	template<typename U>
	TSharedPtr(const TSharedPtr<U>& rhs) {
		Set(static_cast<T*>(rhs._ptr));
	}

	~TSharedPtr() {
		Release();
	}

public:
	//복사 연산자
	TSharedPtr& operator=(const TSharedPtr& rhs) {
		if (_ptr != rhs._ptr) {
			Release();
			Set(rhs._ptr);
		}
		return *this;
	}

	//이동 연산자
	TSharedPtr& operator=(TSharedPtr&& rhs) {
		Release();
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
		return *this;
	}
	
	bool operator==(const TSharedPtr& rhs) const { return _ptr = rhs._ptr; }
	bool operator!=(const TSharedPtr& rhs) const { return _ptr != rhs._ptr; }
	bool operator < (const TSharedPtr& rhs) const { return _ptr < ptr; }
	
	bool operator==(T* ptr) const { return _ptr = ptr; }
	bool operator != (T* ptr) const { return _ptr != ptr; }
	bool operator < (T* ptr) const { return _ptr < ptr; }

	T* operator*() {
		return _ptr;
	}

	const T* operator*() const {
		return _ptr;
	}

	operator T* () const {
		return _ptr;
	}

	T* operator->() () {
		return _ptr;
	}
	const T* operator->() const {
		return _ptr;
	}

	
private:
	inline void Set(T* ptr) {
		_ptr = ptr;
		if (ptr)
			ptr->AddRef();
	}

	inline void Release() {
		if (_ptr != nullptr) {
			_ptr->ReleaseRef();
			_ptr = nullptr;
		}
	}

private :
	T* _ptr = nullptr;
};