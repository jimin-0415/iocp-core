#pragma once

/// <summary>
/// PCH 파일은 GameServer, DummyClient 에서 참조 불가능한 파일 이기때문에 해당 파일을 참조하기 위해서
/// 코어 파일을 생성  
/// </summary>

#include "Types.h"

#include "CoreGlobal.h"
#include "CoreTLS.h"
#include "CoreMacro.h"

#include <vector>
#include <list>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include <Windows.h>
#include <memory>
#include <iostream>
#include <utility>
using namespace std;

#include "Lock.h"

void HelloWorld();