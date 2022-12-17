#pragma once

/// <summary>
/// PCH 파일은 GameServer, DummyClient 에서 참조 불가능한 파일 이기때문에 해당 파일을 참조하기 위해서
/// 코어 파일을 생성  
/// </summary>

#include "Types.h"

#include "CoreGlobal.h"
#include "CoreTLS.h"
#include "CoreMacro.h"
#include "Container.h"

#include <Windows.h>
#include <memory>
#include <iostream>
#include <utility>
using namespace std;

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "Lock.h"
#include "ObjectPool.h"
#include "TypeCast.h"
#include "Memory.h"
#include "SendBuffer.h"
#include "Session.h"
//#define _STOMP

void HelloWorld();