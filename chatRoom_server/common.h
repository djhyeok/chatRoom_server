#pragma once
#ifndef COMMON_H
#define COMMON_H

#pragma warning(disable:4996)
#include <windows.h>

//��Ĺ��� ȯ�溯��
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

//userdata ����ü
typedef struct userdata {
	char userName[200];		//�����̸�
	HANDLE hThread;			//������ �ڵ�
	DWORD ThreadID;			//������ ID
	SOCKET clientsock;		//��Ĺ ID
}USERDATA;

//��� ����ü(LinkedList)
typedef struct userList {
	USERDATA data;
	struct userList* link;	//���������� ����Ű�� ��ũ
}USERNODE;

//roomdata ����ü
typedef struct roomdata {
	unsigned int roomNum;	//�� ������ȣ
	char roomName[200];		//�� �̸�
	USERNODE* roomUserHead;	//�濡 �ִ� ���� ����Ʈ
}ROOMDATA;

//ä�ù濡 ���� �ο� ����ü(LinkedList)
typedef struct roomList {
	ROOMDATA data;			//�� ������ ����ü
	struct roomList* link;	//�������� ����Ű�� ��ũ
}ROOMNODE;

//������ �Ű������� �ѱ� ������ ����ü
typedef struct threadData {
	USERNODE* userHead;
	ROOMNODE* roomHead;
	SOCKET sock;
}THREADDATA;
#endif