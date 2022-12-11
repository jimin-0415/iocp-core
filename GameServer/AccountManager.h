#pragma once

class AccountManager
{
	USE_LOCK

public:
	void AccountThenPlayer();
	void Lock();

private:

};

extern AccountManager GAccountManager;
