#pragma once
#pragma once
#ifndef MYSOCKET_H
#define MYSOCKET_H

void cutMsg(char*, char*);
DWORD WINAPI AccThreadFunc(LPVOID);
DWORD WINAPI RecvThreadFunc(LPVOID);

#endif