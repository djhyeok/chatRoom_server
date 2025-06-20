#pragma once
#ifndef COMMON_H
#define COMMON_H

#pragma warning(disable:4996)
#include <windows.h>

//소캣통신 환경변수
const int buflen = 4096;
typedef struct env {
	WSADATA wsadata;
	SOCKET listensock;
	sockaddr_in addr_server;
	sockaddr_in addr_client;
	int addrlen_srv = sizeof(sockaddr);
	int addrlen_clt = sizeof(sockaddr);
	in_addr in;
	hostent* ht;
	char buf[buflen];
	char strTemp[buflen];
}ENV;

//userdata 구조체
typedef struct userdata {
	char userName[200];		//유저이름
	HANDLE hThread;			//스레드 핸들
	DWORD ThreadID;			//스레드 ID
	SOCKET clientsock;		//소캣 ID
}USERDATA;

//노드 구조체(LinkedList)
typedef struct userList {
	USERDATA data;
	struct userList* link;	//다음유저를 가리키는 링크
}USERNODE;

//roomdata 구조체
typedef struct roomdata {
	unsigned int roomNum;	//방 고유번호
	char roomName[200];		//방 이름
	USERNODE* roomUserHead;	//방에 있는 유저 리스트
}ROOMDATA;

//채팅방에 속한 인원 구조체(LinkedList)
typedef struct roomList {
	ROOMDATA data;			//방 데이터 구조체
	struct roomList* link;	//다음방을 가리키는 링크
}ROOMNODE;

//스레드 매개변수로 넘길 데이터 구조체
typedef struct threadData {
	USERNODE* userHead;
	ROOMNODE* roomHead;
	SOCKET sock;
}THREADDATA;
#endif