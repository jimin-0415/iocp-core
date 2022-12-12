#pragma once

#include "Allocator.h"

template<typename Type, typename... Args>
Type* xnew(Args&&... args) {
	Type* memory = static_cast<Type*>(XAlloc(sizeof(Type)));

	//placement new
	new(memory)Type(forward<Args>(args)...); //xvalue, rvalue 일경우 foward 를 통해서 넘겨준다.
	return memory;
}

template<typename Type>
void xdelete(Type* obj) {
	obj->~Type();	//객체를 대상으로 소멸자를 호출한다.
	XRelease(obj);	//메모리를 반납한다.
}

//template<typename Type, typename... Args>
//Type* xnew(Args&&... args)
//{
//	Type* memory = static_cast<Type*>(xalloc(sizeof(Type)));
//	new(memory)Type(forward<Args>(args)...); // placement new
//	return memory;
//}
//
//template<typename Type>
//void xdelete(Type* obj)
//{
//	obj->~Type();
//	xrelease(obj);
//}