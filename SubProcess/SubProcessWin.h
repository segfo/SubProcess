#pragma once
#include<Windows.h>
#include "SubProcess.h"
#include "tstring.h"
class SubProcessWin : public SubProcess {
private:
	tstring exeFilePath;
	tstring cmdOpt;
	STARTUPINFO startInfo;
	PROCESS_INFORMATION procInfo;
	SECURITY_ATTRIBUTES secAttr;
	HANDLE stdInput;
	HANDLE stdOutput;
	void apiFailed (string apiName, DWORD errCode);
public:
	SubProcessWin ();
	~SubProcessWin ();
	void setModuleInfo (string exeFilePath,string cmdOpt);
	void createProcess ();
	void setRedirect ();
	size_t readTTY(uint8_t *buffer, int length);
	size_t writeTTY(const uint8_t *buffer, int length);
	pid_t getPid ();
	void resumeProcess ();
	void suspendProcess ();
	intmax_t waitProcess ();
	void killProcess ();
};
