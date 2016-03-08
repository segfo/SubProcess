#pragma once
#ifndef __SUB_PROCESS_H__
#define __SUB_PROCESS_H__
#include<iostream>

typedef unsigned long pid_t;

using namespace std;
class SubProcess {
public:
	SubProcess(){}
	virtual ~SubProcess() {}
	virtual void setModuleInfo (string exeFilePath, string cmdOpt) = 0;
	virtual void createProcess () = 0;
	virtual void setRedirect () = 0;
	virtual size_t readTTY (uint8_t *buffer,int length) = 0;
	virtual size_t writeTTY (const uint8_t *buffer, int length) = 0;
	virtual pid_t getPid () = 0;
	virtual void resumeProcess () = 0;
	virtual void suspendProcess () = 0;
	virtual intmax_t waitProcess () = 0;
	virtual void killProcess () = 0;
};

#endif