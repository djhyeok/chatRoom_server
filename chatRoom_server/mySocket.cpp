#include "common.h"
#include "mySocket.h"
#include "linkedList.h"
#include <commctrl.h>
#include <stdio.h>

extern ENV g_env;
extern HWND g_hRoomList;
extern HWND g_hWndMain;
extern int g_Fnum;
extern HANDLE hSemaphore;

/*
void cutMsg(char* msg,char* buffer): buffer���� �޽��� �߶� msg�� ����
*/
void cutMsg(char* msg, char* buffer) {
	int i = 1, j = 0;

	while (buffer[i] != '\0') {
		msg[j++] = buffer[i++];
	}
	msg[j] = '\0';
	return;
}

/*
DWORD WINAPI AccThreadFunc(LPVOID Param): accept()�� �����ϸ� recv Thread�� �����ϴ� accept Thread�Լ�
*/
DWORD WINAPI AccThreadFunc(LPVOID Param) {
	THREADDATA P = *(THREADDATA*)Param;
	USERDATA userdata;
	THREADDATA threadParam;
	for (;;) {
		userdata.clientsock = accept(P.sock, (sockaddr*)&g_env.addr_client, &g_env.addrlen_clt);
		if (userdata.clientsock == INVALID_SOCKET) {
			break;
		}
		else {
			SendMessage(g_hRoomList, LB_ADDSTRING, 0, (LPARAM)"Ŭ���̾�Ʈ�� �����Ͽ����ϴ�.");
			//usernode �߰�,recv ������ ���� �� userList ���ΰ�ħ �� ������
			threadParam.sock = userdata.clientsock;
			threadParam.userHead = P.userHead;
			threadParam.roomHead = P.roomHead;
			userdata.hThread = CreateThread(NULL, 0, RecvThreadFunc, &threadParam, 0, &(userdata.ThreadID));
			addUserNode(P.userHead, userdata);
		}
	}
	return 0;
}

/*
DWORD WINAPI RecvThreadFunc(LPVOID Param): ���ú� ������ �Լ�
*/
DWORD WINAPI RecvThreadFunc(LPVOID Param) {
	LVITEM LI;
	int nReturn;
	THREADDATA P = *(THREADDATA*)Param;
	USERNODE* userNode = findUserNode(P);
	USERNODE* tempUser;
	ROOMNODE* roomNode = NULL;
	ROOMNODE* tempRoom;
	char strTemp[200], strTemp2[200];
	char msgTemp[1024];
	int num, imgSize, recvSize;
	FILE* fp;

	for (;;) {
		nReturn = recv(P.sock, g_env.buf, buflen, 0);
		if (nReturn > 0) {				//������ ������ ���� ���
			//������� ��û			"R�����̸�"
			if (g_env.buf[0] == 'R') {
				//�ش� ��Ĺ�� ���� ��带 ã�� �����̸� ����
				cutMsg(userNode->data.userName, g_env.buf);
				showUserList(P.userHead);
			}
			//ä�ù渮��Ʈ ��û		"I"
			else if (g_env.buf[0] == 'I') {
				tempRoom = P.roomHead;
				//��û�� client���� ��� ���� ������ "c���ȣ|���̸�" send �ؼ� client���� �߰��� �� �ְ� ��
				while (tempRoom->link) {
					tempRoom = tempRoom->link;
					wsprintf(msgTemp, "c%d|%s", tempRoom->data.roomNum, tempRoom->data.roomName);
					nReturn = send(P.sock, msgTemp, sizeof(msgTemp), 0);
				}
			}
			//ä�ù� ������û		"Cä�ù��̸�"
			else if (g_env.buf[0] == 'C') {
				//�ش� ��Ĺ�� ���� ������带 ã�� ���̸� �Ľ� �� �� ����
				cutMsg(strTemp, g_env.buf);
				num = addRoomNode(P.roomHead, strTemp, userNode->data);
				//������ �� ����(�� ����,�� ��ȣ) ä�ù� ����Ʈ�信 �Ѹ�
				LI.mask = LVIF_TEXT;
				LI.iItem = 0;
				LI.iSubItem = 0;
				LI.pszText = strTemp;
				ListView_InsertItem(g_hRoomList, &LI);
				wsprintf(strTemp, "%d", num);
				ListView_SetItemText(g_hRoomList, 0, 1, strTemp);
				//������ �� ������ ��� Ŭ���̾�Ʈ�� ����
				roomNode = findRoomNode(P.roomHead, num);
				tempUser = P.userHead;
				while (tempUser->link) {
					tempUser = tempUser->link;
					//��� Ŭ���̾�Ʈ�� c���ȣ|���̸� send
					wsprintf(msgTemp, "c%s|%s", strTemp, roomNode->data.roomName);
					nReturn = send(tempUser->data.clientsock, msgTemp, sizeof(msgTemp), 0);
					//����� ��û�� ���� ����ڰ� �ش� �� ������ �� �ְ� send
					if (tempUser == userNode) {
						//�� ���� �޽��� p���ȣ send
						wsprintf(msgTemp, "p%s", strTemp);
						nReturn = send(userNode->data.clientsock, msgTemp, sizeof(msgTemp), 0);
					}
				}
			}
			//ä�ù� ������û		"P���ȣ"
			else if (g_env.buf[0] == 'P') {
				//strTemp�� ���ȣ ����
				cutMsg(strTemp, g_env.buf);
				//�ش� ���ȣ�� ��ġ�ϴ� �� roomNode�� �����ϰ� �ش� �濡 userHead�� �������� �߰�
				roomNode = findRoomNode(P.roomHead, atoi(strTemp));
				addUserNode(roomNode->data.roomUserHead, userNode->data);
				//�� ���� �޽��� p���ȣ send
				wsprintf(msgTemp, "p%s", strTemp);
				nReturn = send(P.sock, msgTemp, sizeof(msgTemp), 0);
			}
			//�޽��� ���޿�û		"M�޽���"
			else if (g_env.buf[0] == 'M') {
				cutMsg(strTemp, g_env.buf);
				//tempUser�� ���� �� ������ LinkedList ��� ����
				tempUser = roomNode->data.roomUserHead;
				//�޽��� ����     "M�������:�޽���"
				wsprintf(msgTemp, "m%s:%s", userNode->data.userName, strTemp);
				while (tempUser->link) {
					tempUser = tempUser->link;
					//���ο��� ���� ���
					if (tempUser->data.clientsock == userNode->data.clientsock) {
						wsprintf(strTemp2, "m(��):%s", strTemp);
						nReturn = send(tempUser->data.clientsock, strTemp2, sizeof(strTemp2), 0);
					}
					//������ �ƴ� ���
					else {
						nReturn = send(tempUser->data.clientsock, msgTemp, sizeof(msgTemp), 0);
					}
				}
			}
			//���� ���� ��û "F����ũ��"
			else if (g_env.buf[0] == 'F') {
				//���� ���� �� �� Ŭ���̾�Ʈ�� ���������� �ٸ� Ŭ���̾�Ʈ�� ���
				WaitForSingleObject(hSemaphore, INFINITE);
				cutMsg(strTemp, g_env.buf);
				imgSize = atoi(strTemp);
				lstrcpy(msgTemp, g_env.buf);
				msgTemp[0] = 'f';
				////���� �濡 �����ڵ鿡�� f����ũ�� send
				tempUser = roomNode->data.roomUserHead;
				while (tempUser->link) {
					tempUser = tempUser->link;
					send(tempUser->data.clientsock, msgTemp, lstrlen(msgTemp), 0);
				}
				recvSize = 0;
				while (recvSize < imgSize) {
					nReturn = recv(P.sock, msgTemp, 1024, 0);
					if (nReturn < 0) {
						break;
					}
					tempUser = roomNode->data.roomUserHead;
					while (tempUser->link) {
						tempUser = tempUser->link;
						send(tempUser->data.clientsock, msgTemp, 1024, 0);
					}
					recvSize += 1024;
				}
				ReleaseSemaphore(hSemaphore, 1, NULL);
			}
			//ä�ù� �������û		"E���ȣ"
			else if (g_env.buf[0] == 'E') {
				cutMsg(strTemp, g_env.buf);
				num = atoi(strTemp);
				//���ȣ�� �ش��ϴ� �� ��� Ž��
				roomNode = findRoomNode(P.roomHead, num);
				//�� ���� �ο� LinkedList���� ���� ����
				deleteUserNode(roomNode->data.roomUserHead, P.sock);
				//���� ������ ������ �������� ���
				if (roomNode->data.roomUserHead->link == NULL) {
					//ä�ù� �������û,����
					wsprintf(msgTemp, "e%d", roomNode->data.roomNum);
					//�ش� ���ȣ�� ���� �� ����
					deleteRoomNode(P.roomHead, num);
					tempUser = P.userHead;
					//������ ���� �ٸ� ����ڿ����� �����Ҽ� �ְ� send
					while (tempUser->link) {
						tempUser = tempUser->link;
						nReturn = send(tempUser->data.clientsock, msgTemp, sizeof(msgTemp), 0);
					}
					//ä�ù� ��� ������ ä�ù� ����Ʈ ���ΰ�ħ
					showRoomList(P.roomHead);
				}
				//���� ������ �������� �ƴѰ��
				else {
					//ä�ù� ������ ��û �Ϸ� "e"
					nReturn = send(P.sock, "e", sizeof("e"), 0);
				}
			}
		}
		else if (nReturn == 0) {		//Ŭ���̾�Ʈ ���� ����(Ŭ���̾�Ʈ���� shutdown)
			deleteUserNode(P.userHead, P.sock);
			showUserList(P.userHead);
			shutdown(P.sock, 1);
			closesocket(P.sock);
			ExitThread(0);
		}
		else {							//������ ����
			MessageBox(g_hWndMain, "���ſ���", "���ſ���", MB_OK);
			break;
		}
	}
	return 0;
}