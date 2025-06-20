#include "common.h"
#include "linkedList.h"
#include "mySocket.h"
#include <commctrl.h>

extern HWND g_hWndMain, g_hRoomList, g_hUserList;

/*
USERNODE* createUserNode():user node�� �Ҵ�
*/
USERNODE* createUserNode() {
	USERNODE* temp;
	temp = (USERNODE*)malloc(sizeof(USERNODE));
	temp->link = NULL;
	return temp;
}

/*
void addUserNode(USERNODE* head,USERDATA data): head�� ��ũ�� data���� �ʱⰪ���� userNode �߰�
*/
void addUserNode(USERNODE* head, USERDATA data) {
	USERNODE* t = createUserNode();
	t->data = data;
	t->link = head->link;
	head->link = t;

	return;
}

/*
void deleteUserNode(USERNODE* head,SOCKET clsock): head�� ��ũ�� �� socket�� ������ socket ������ ���� userNode ����
*/
void deleteUserNode(USERNODE* head, SOCKET clsock) {
	USERNODE* temp = head;
	USERNODE* temp2 = head;
	while (temp->link) {
		temp2 = temp;
		temp = temp->link;
		if (temp->data.clientsock == clsock) {
			temp2->link = temp->link;
			free(temp);
			break;
		}
	}
	return;
}

/*
void deleteAllUserNode(USERNODE* head): head�� ��ũ�� ����� ��� ���� ��� ����
*/
void deleteAllUserNode(USERNODE* head) {
	USERNODE* temp;
	while (head->link) {
		temp = head->link;
		head->link = temp->link;
		free(temp);
	}
	return;
}

/*
USERNODE* findUserNode(THREADDATA P): P.userHead �� ��ũ�� ����� ��� �� P.sock�� ���� ��Ĺ������ ���� node ����
*/
USERNODE* findUserNode(THREADDATA P) {
	USERNODE* temp = P.userHead;
	while (temp->link) {
		temp = temp->link;
		if (P.sock == temp->data.clientsock) {
			return temp;
		}
	}
	return NULL;
}

/*
ROOMNODE* createRoomNode(): room node�� �Ҵ�
*/
ROOMNODE* createRoomNode() {
	ROOMNODE* temp;
	temp = (ROOMNODE*)malloc(sizeof(ROOMNODE));
	temp->link = NULL;
	return temp;
}

/*
int addRoomNode(ROOMNODE* roomHead,char* roomName,USERDATA uData): roomName �̸����� roomHead �ڿ� roomNode �߰��ϰ�
																	������� ���ÿ� ������ ���� ������ uData �ϳ� �߰� �� ������ ���ȣ ����
*/
int addRoomNode(ROOMNODE* roomHead, char* roomName, USERDATA uData) {
	char tempStr[10];
	//�� ��� �Ҵ��� ������ �ְ� ��ũ ����
	ROOMNODE* temp = createRoomNode();
	lstrcpy(temp->data.roomName, roomName);
	if (roomHead->link) {
		temp->link = roomHead->link;
		temp->data.roomNum = roomHead->link->data.roomNum + 1;
	}
	else {
		temp->data.roomNum = 1;
	}
	roomHead->link = temp;
	//�ش� ���� ��������Ʈ ��� �Ҵ� �� ������ ������� �߰�
	temp->data.roomUserHead = createUserNode();
	addUserNode(temp->data.roomUserHead, uData);

	return temp->data.roomNum;
}

/*
void deleteRoomNode(ROOMNODE* roomHead,unsigned int num): �Ű������� ���� num�� ������ roomNum ������ ���� roomNode ����
*/
void deleteRoomNode(ROOMNODE* roomHead, unsigned int num) {
	ROOMNODE* temp = roomHead;
	ROOMNODE* temp2 = roomHead;

	while (temp->link) {
		temp2 = temp;
		temp = temp->link;
		if (temp->data.roomNum == num) {
			temp2->link = temp->link;
			free(temp->data.roomUserHead);
			free(temp);
			break;
		}
	}
	return;
}

/*
void deleteAllRoomNode(ROOMNODE* roomHead): �� ��� ��� ����
*/
void deleteAllRoomNode(ROOMNODE* roomHead) {
	while (roomHead->link) {
		deleteRoomNode(roomHead, roomHead->link->data.roomNum);
	}
	return;
}

/*
void showRoomList(ROOMNODE* roomHead): �� ��� ���ΰ�ħ
*/
void showRoomList(ROOMNODE* roomHead) {
	ROOMNODE* temp = roomHead;
	LVITEM LI;
	char tempStr[200];
	//����Ʈ�� ��� ����� ���ΰ�ħ
	ListView_DeleteAllItems(g_hRoomList);
	while (temp->link) {
		temp = temp->link;
		//LV�� ���̸� ���� �߰�
		LI.mask = LVIF_TEXT;
		LI.iItem = 0;
		LI.iSubItem = 0;
		LI.pszText = temp->data.roomName;
		ListView_InsertItem(g_hRoomList, &LI);
		wsprintf(tempStr, "%d", temp->data.roomNum);
		//LV�� ���ȣ ���� �߰�
		ListView_SetItemText(g_hRoomList, 0, 1, tempStr);
	}
	return;
}

/*
void showUserList(USERNODE* userHead): ���� ��� ���ΰ�ħ
*/
void showUserList(USERNODE* userHead) {
	USERNODE* temp = userHead;
	SendMessage(g_hUserList, LB_RESETCONTENT, 0, 0);
	while (temp->link) {
		temp = temp->link;
		SendMessage(g_hUserList, LB_ADDSTRING, 0, (LPARAM)temp->data.userName);
	}
	return;
}

/*
ROOMNODE* findRoomNode(ROOMNODE* roomHead,int roomNum): ���ȣ�� roomNum�� �� ã�Ƽ� ����
*/
ROOMNODE* findRoomNode(ROOMNODE* roomHead, int roomNum) {
	ROOMNODE* temp = roomHead;
	while (temp->link) {
		temp = temp->link;
		if (temp->data.roomNum == roomNum) {
			return temp;
		}
	}
	return NULL;
}

void printUserNode(USERNODE* head) {
	USERNODE* temp = head;
	char tt[40];
	char str[200] = "";
	while (temp->link != NULL) {
		temp = temp->link;
		wsprintf(tt, "%d ", temp->data.clientsock);
		lstrcat(str, tt);
	}
	MessageBox(g_hWndMain, str, NULL, MB_OK);
	return;
}