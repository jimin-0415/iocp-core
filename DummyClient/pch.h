#pragma once

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

//Linker ���� �ʰ� pch�� ����
#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib")
#else 
#pragma comment(lib, "Release\\ServerCore.lib")
#endif

//library ���� �߰�
#include "CorePch.h"