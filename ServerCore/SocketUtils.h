#pragma once
#include "NetAddress.h"

/// <summary>
/// Socket Util
/// </summary>
class SocketUtils 
{
public:
	static LPFN_CONNECTEX ConnectEx;
	static LPFN_DISCONNECTEX DisconnectEx;
	static LPFN_ACCEPTEX AcceptEx;
public:
	static void Init();
	static void Clear();

	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
	static SOCKET CreateSocket();

	static bool SetLinger(SOCKET socket, uint32 onoff, uint32 linger);
	static bool SetReuseAddress(SOCKET socket, bool flag);
	static bool SetRecvBufferSize(SOCKET socket, int32 size);
	static bool SetSendBufferSize(SOCKET socket, int32 size);
	static bool SetTcpNoDelay(SOCKET socket, bool flag);					//Nagel Algorithm 
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);	//listenSocket의 특성을 Client속성에 전파한다.

	static bool Bind(SOCKET socket, NetAddress netAddr);
	static bool BindAnyAddress(SOCKET socket, uint16 port);
	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
	static bool Close(SOCKET socket);	
};

//SetSocket 옵션을 매핑해서 할 수 있도록 한다.
template<typename T>
static inline bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal) {
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<const char*>(&optVal), sizeof(T));
}