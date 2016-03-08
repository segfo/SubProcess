#include<stdio.h>
#include<windows.h>
#include<tchar.h>
#include"SubProcess.h"
#include"SubProcessWin.h"

int	main(){
	SubProcess *proc = new SubProcessWin();
	uint8_t buf[0x1000] = {0};
	try {
		proc->setModuleInfo("C:\\windows\\system32\\cmd.exe", "");
		proc->setRedirect();
		proc->createProcess();
		proc->resumeProcess();
		proc->writeTTY((uint8_t*)"ipconfig\r\n",10);
		Sleep(3000);
		proc->killProcess();
		size_t readBytes = proc->readTTY(buf, sizeof(buf)-1);
		printf("bytes: %d\n", readBytes);
		printf("%s\n",buf);
		intmax_t ret = proc->waitProcess();
	}
	catch (runtime_error &e) {
		printf("%s\n",e.what());
	}
	delete proc;
	return 0;
}
