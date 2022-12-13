#pragma once 
#include "Types.h"

//list li { Player, Knight, Mage} 이렇게 넣는다.

#pragma region TypeList

//인자를 여러개 줄 수 있도록 한다.
template<typename... T>
struct TypeList;

//데이터가 두개면 이 템플릿을 선택한다.
template<typename T, typename U>
struct TypeList<T, U>
{
	using Head = T;
	using Tail = U;
};

//데이터가 3개 이상이면 재귀적으로 탄다.
template<typename T, typename... U>
struct TypeList<T, U...>
{
	using Head = T;
	using Tail = TypeList<U...>;
};

//TypeList<A, B>::Head head [A];
//TypeList<A, B>::Tail tail [B];
//
//TypeList<A, TypeList<B, C>>::Head head [A];
//TypeList<A, TypeList<B, C>>::Tail::Head tail1 [B];
//TypeList<A, TypeList<B, C>>::Tail::Tail tail2 [C];
//템플릿은 컴파일 타이밍에 모두 정의한다.

#pragma endregion

#pragma region Length

template<typename T>
struct Length;

template<>
struct Length<TypeList<>> 
{
	enum { value = 0, };
};

template<typename T, typename... U>
struct Length<TypeList<T, U...>>
{
	enum{ value = 1 + Length<TypeList<U...>>::value, };
};

//int32 len1 = Length<TypeList<Player, Monster, Knight>>::value; <- 컴파일 단계에서 몇개인지 안다.
// 1+ 1+ 1+ 0[빈TypeList<>] 호출

#pragma endregion

#pragma region TypeAt
template<typename TL, int32 index>
struct TypeAt;

//index가 0일 경우Head
template<typename Head, typename... Tail>
struct TypeAt<TypeList<Head, Tail...>, 0>
{
	using Result = Head;
};

//Type At 을 하나씩 줄이면서 찾는다. 제귀
template<typename Head, typename... Tail, int32 index>
struct TypeAt<TypeList<Head, Tail...>, index>
{
	using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
};

//TypeList<A, B, C> index = 2는 누구냐		struct TypeAt<TypeList<Head, Tail...>, index>
//TypeList<B, C> index 1					struct TypeAt<TypeList<Head, Tail...>, index>
//TypeList<C> index 0 -> C 결과가 나옴		TypeAt<TypeList<Head, Tail...>, 0>

#pragma endregion

#pragma region IndexOf
//몇번째 인덱스냐

template<typename TL, typename T>
struct IndexOf;

template<typename... Tail, typename T>
struct IndexOf<TypeList<T, Tail...>, T>
{
	enum { value = 0 };
};

//못찾을 경우-1 
template<typename T>
struct IndexOf<TypeList<>, T>
{
	enum { value = -1 };
};

template<typename Head, typename... Tail, typename T>
struct IndexOf<TypeList<Head, Tail...>, T>
{
private:
	enum { temp = IndexOf<TypeList<Tail...>, T>::value };
public:
	enum { value = (temp == -1) ? -1 : temp + 1 };
};

#pragma endregion

#pragma region Conversion
template<typename From, typename To>
class Conversion
{
private:
	using Small = __int8;
	using Big = __int32;

	static Small Test(const To&) { return 0; }
	static Big Test(...) { return 0; }
	static From MakeFrom() { return 0; }

public:
	enum
	{
		exists = sizeof(Test(MakeFrom())) == sizeof(Small)
	};
};
#pragma endregion

#pragma region TypeCast

template<int32 v>
struct Int2Type
{
	enum { value = v };
};

template<typename TL>
class TypeConversion
{
public:
	enum
	{
		length = Length<TL>::value
	};

	TypeConversion()
	{
		MakeTable(Int2Type<0>(), Int2Type<0>());
	}

	template<int32 i, int32 j>
	static void MakeTable(Int2Type<i>, Int2Type<j>)
	{
		using FromType = typename TypeAt<TL, i>::Result;
		using ToType = typename TypeAt<TL, j>::Result;

		if (Conversion<const FromType*, const ToType*>::exists)
			s_convert[i][j] = true;
		else
			s_convert[i][j] = false;

		MakeTable(Int2Type<i>(), Int2Type<j + 1>());
	}

	template<int32 i>
	static void MakeTable(Int2Type<i>, Int2Type<length>)
	{
		MakeTable(Int2Type<i + 1>(), Int2Type<0>());
	}

	template<int j>
	static void MakeTable(Int2Type<length>, Int2Type<j>)
	{
	}

	static inline bool CanConvert(int32 from, int32 to)
	{
		static TypeConversion conversion;
		return s_convert[from][to];
	}

public:
	static bool s_convert[length][length];
};

template<typename TL>
bool TypeConversion<TL>::s_convert[length][length];

template<typename To, typename From>
To TypeCast(From* ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_cast<To>(ptr);

	return nullptr;
}


template<typename To, typename From>
shared_ptr<To> TypeCast(shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return nullptr;

	using TL = typename From::TL;

	if (TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value))
		return static_pointer_cast<To>(ptr);

	return nullptr;
}

template<typename To, typename From>
bool CanCast(From* ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value);
}


template<typename To, typename From>
bool CanCast(shared_ptr<From> ptr)
{
	if (ptr == nullptr)
		return false;

	using TL = typename From::TL;
	return TypeConversion<TL>::CanConvert(ptr->_typeId, IndexOf<TL, remove_pointer_t<To>>::value);
}

#pragma endregion

#define DECLARE_TL		using TL = TL; int32 _typeId;
#define INIT_TL(Type)	_typeId = IndexOf<TL, Type>::value;