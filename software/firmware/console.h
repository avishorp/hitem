// Console Functions

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "simplelink.h"

void ConsoleInit();
void ConsolePrint(const char* pStr);
int ConsolePrintf(const char *pcFormat, ...);
void ConsoleProcessTX();
void ConsoleConnectUDP(_u32 ip, _u16 port);
void ConsoleDisconnectUDP();

#endif

