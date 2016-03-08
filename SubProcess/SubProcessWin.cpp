#include "SubProcessWin.h"

SubProcessWin::SubProcessWin () {
	this->stdInput = nullptr;
	this->stdOutput = nullptr;
    this->secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	this->secAttr.lpSecurityDescriptor = NULL;
	this->secAttr.bInheritHandle = TRUE;
	memset(&startInfo, 0, sizeof(startInfo));
	this->startInfo.cb = sizeof(startInfo);
	this->startInfo.dwFlags = STARTF_USESTDHANDLES;
	this->startInfo.wShowWindow = SW_SHOWDEFAULT;
	this->startInfo.hStdInput = nullptr;
	this->startInfo.hStdOutput = nullptr;
	this->startInfo.hStdError = nullptr;
}

SubProcessWin::~SubProcessWin () {
	if ( this->stdInput == nullptr) {
		CloseHandle(this->stdInput);
	}
	if ( this->stdOutput == nullptr) {
		CloseHandle(this->stdOutput);
	}
	if ( this->startInfo.hStdInput == nullptr) {
		CloseHandle(this->startInfo.hStdInput);
	}
	if ( this->startInfo.hStdOutput == nullptr) {
		CloseHandle(this->startInfo.hStdOutput);
	}
	if ( this->startInfo.hStdError == nullptr) {
		CloseHandle(this->startInfo.hStdError);
	}
}

void SubProcessWin::setModuleInfo ( string exeFilePath, string cmdOpt) {
#ifdef UNICODE
	int result;
	size_t requireSize;

	if ( exeFilePath.length () != 0 ) {
		requireSize = MultiByteToWideChar (CP_UTF8, 0, exeFilePath.c_str (), -1, ( wchar_t * ) nullptr, 0);
		WCHAR *exeFilePathW = new WCHAR[requireSize];
		if ( exeFilePathW == nullptr ) {
			return;
		}
		result = MultiByteToWideChar (CP_ACP, 0, exeFilePath.c_str (), exeFilePath.length (), exeFilePathW, requireSize);
		if ( result == 0 ) {
			apiFailed ("MultiByteToWideChar (Execute module filepath)", GetLastError ());
		}
		exeFilePathW[requireSize - 1] = _T ('\0');
		this->exeFilePath = exeFilePathW;
		delete[] exeFilePathW;
	}
	//////////////////////////////
	if ( cmdOpt.length () != 0 ) {
		requireSize = MultiByteToWideChar (CP_UTF8, 0, cmdOpt.c_str (), -1, ( wchar_t * ) nullptr, 0);
		WCHAR *cmdOptW = new WCHAR[requireSize];
		if ( cmdOptW == nullptr ) {
			return;
		}
		result = MultiByteToWideChar (CP_ACP, 0, cmdOpt.c_str (), cmdOpt.length (), cmdOptW, requireSize);
		if ( result == 0 ) {
			apiFailed ("MultiByteToWideChar (Command line options)", GetLastError ());
		}
		cmdOptW[requireSize - 1] = _T ('\0');
		this->cmdOpt = cmdOptW;
		delete[] cmdOptW;
	}
#else
	this->execDir = execDir;
	this->exeFilePath = exeFilePath;
	this->cmdOpt = cmdOpt;
#endif
}

void SubProcessWin::apiFailed (string apiName,DWORD errCode) {
	TCHAR errmsg[512];
	char msg[512];
	FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, NULL,errCode, 0, errmsg, sizeof (errmsg), NULL);
	_tprintf (_T ("%s"), errmsg);
	snprintf (msg,sizeof(msg),"%s(win32) fail.",apiName.c_str());
	throw runtime_error (msg);
}

void SubProcessWin::createProcess () {
	BOOL result = CreateProcess (this->exeFilePath.c_str(), (TCHAR*) this->cmdOpt.c_str(), NULL, NULL,
				 TRUE, CREATE_SUSPENDED, NULL,NULL,
				   &this->startInfo, &this->procInfo);
	if ( result == FALSE ) {
		apiFailed ("CreateProcess", GetLastError ());
	}
}

void SubProcessWin::setRedirect () {
	HANDLE stdOutput_r,stdOutput_w;
	HANDLE stdError_w; 
	HANDLE stdInput_r,stdInput_w;
	HANDLE proc = GetCurrentProcess();

	// 標準出力、標準入力パイプの両端を作成する
	CreatePipe(&stdOutput_r, &stdOutput_w,&this->secAttr,0);
	CreatePipe(&stdInput_r,&stdInput_w,&this->secAttr,0);
	// 標準出力のパイプ端を標準エラー出力のパイプ端としても使う
	DuplicateHandle(proc,stdOutput_w,proc,&stdError_w,0,TRUE, DUPLICATE_SAME_ACCESS);
	// 受け側を親プロセスで保持
	this->stdInput = stdInput_w;
	this->stdOutput = stdOutput_r;
	// 子プロセスへ継承
	this->startInfo.hStdInput = stdInput_r;
	this->startInfo.hStdOutput = stdOutput_w;
	this->startInfo.hStdError = stdError_w;
}

size_t SubProcessWin::readTTY(uint8_t *buffer, int length) {
	size_t readBytes;
	ReadFile(this->stdOutput, buffer, length, (LPDWORD)&readBytes, NULL);
	return readBytes;
}

size_t SubProcessWin::writeTTY (const uint8_t *buffer, int length) {
	size_t writeBytes;
	WriteFile(this->stdInput, buffer, length, (LPDWORD)&writeBytes, NULL);
	return writeBytes;
}

pid_t SubProcessWin::getPid () {
	return this->procInfo.dwProcessId;
}

void SubProcessWin::resumeProcess () {
	ResumeThread (this->procInfo.hThread);
}

void SubProcessWin::suspendProcess () {
	SuspendThread (this->procInfo.hThread);
}

intmax_t SubProcessWin::waitProcess () {
	DWORD exitCode;
	WaitForSingleObject (this->procInfo.hProcess, INFINITE);
	GetExitCodeProcess (this->procInfo.hProcess, &exitCode);
	return exitCode;
}

void SubProcessWin::killProcess () {
	BOOL result = TerminateProcess (this->procInfo.hProcess,0);
	if ( result == FALSE ) {
		apiFailed ("TerminateProcess",GetLastError());
	}

/*	int exitCodeData = 0;

	void *exitCode = VirtualAllocEx (this->procInfo.hProcess, NULL, sizeof (int), MEM_COMMIT, PAGE_READWRITE);
	if ( exitCode == nullptr ) {
		apiFailed ("VirtualAllocEx", GetLastError ());
	}
	WriteProcessMemory (this->procInfo.hProcess, exitCode, ( void * ) &exitCodeData, sizeof (int), NULL);

	HMODULE kernel32 = GetModuleHandle (_T ("kernel32"));
	if ( kernel32 == nullptr ) {
		apiFailed ("GetModuleHandle", GetLastError ());
	}
	FARPROC exitProcess = GetProcAddress (kernel32, "ExitProcess");
	if ( exitProcess == nullptr ) {
		apiFailed ("GetProcAddress", GetLastError ());
	}
	HANDLE result = CreateRemoteThread (this->procInfo.hProcess, NULL, 0, ( LPTHREAD_START_ROUTINE ) exitProcess, exitCode, 0, NULL);
	if ( result == nullptr ) {
		// 32 bit (WOW64) -> 64 bit (Native) はアドレス空間が違うため失敗する
		apiFailed ("CreateRemoteThread", GetLastError ());
	}*/
}
