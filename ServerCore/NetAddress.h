#pragma once

/// <summary>
/// NetAddress
/// </summary>
class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN sockAddr);
	NetAddress(wstring ip, uint16 port);

	SOCKADDR_IN& GetSockAddr() {
		return _sockAddr;
	}

	wstring GetIpAddress();
	uint16	GetPort() {
		return::ntohs(_sockAddr.sin_port);
	}

public:
	static IN_ADDR Ip2Address(const WCHAR* ip);
	//cdn 주소를 ip 변환도 해당 클래스에 정의.

private:
	SOCKADDR_IN _sockAddr = {};
};

