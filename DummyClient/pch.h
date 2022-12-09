#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

//Linker 넣지 않고 pch에 선엄
#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib")
#else 
#pragma comment(lib, "Release\\ServerCore.lib")
#endif

//library 에서 추가
#include "CorePch.h"