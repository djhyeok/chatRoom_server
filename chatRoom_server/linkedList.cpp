#include "common.h"
#include "linkedList.h"
#include "mySocket.h"
#include <commctrl.h>

extern HWND g_hWndMain, g_hRoomList, g_hUserList;

/*
USERNODE* createUserNode():user node를 할당
*/
USERNODE* createUserNode() {
	USERNODE* temp;
	temp = (USERNODE*)malloc(sizeof(USERNODE));
	temp->link = NULL;
	return temp;
}

/*
void addUserNode(USERNODE* head,USERDATA data): head의 링크에 data값을 초기값으로 userNode 추가
*/
void addUserNode(USERNODE* head, USERDATA data) {
	USERNODE* t = createUserNode();
	t->data = data;
	t->link = head->link;
	head->link = t;

	return;
}

/*
void deleteUserNode(USERNODE* head,SOCKET clsock): head의 링크들 중 socket과 동일한 socket 정보를 가진 userNode 삭제
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
void deleteAllUserNode(USERNODE* head): head의 링크로 연결된 모든 유저 노드 삭제
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
USERNODE* findUserNode(THREADDATA P): P.userHead 의 링크에 연결된 노드 중 P.sock과 같은 소캣정보를 가진 node 리턴
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
ROOMNODE* createRoomNode(): room node를 할당
*/
ROOMNODE* createRoomNode() {
	ROOMNODE* temp;
	temp = (ROOMNODE*)malloc(sizeof(ROOMNODE));
	temp->link = NULL;
	return temp;
}

/*
int addRoomNode(ROOMNODE* roomHead,char* roomName,USERDATA uData): roomName 이름으로 roomHead 뒤에 roomNode 추가하고
																	방생성과 동시에 생성한 유저 데이터 uData 하나 추가 후 생성된 방번호 리턴
*/
int addRoomNode(ROOMNODE* roomHead, char* roomName, USERDATA uData) {
	char tempStr[10];
	//새 노드 할당후 데이터 넣고 링크 연결
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
	//해당 방의 유저리스트 헤드 할당 후 생성한 유저노드 추가
	temp->data.roomUserHead = createUserNode();
	addUserNode(temp->data.roomUserHead, uData);

	return temp->data.roomNum;
}

/*
void deleteRoomNode(ROOMNODE* roomHead,unsigned int num): 매개변수로 받은 num과 동일한 roomNum 정보를 가진 roomNode 삭제
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
void deleteAllRoomNode(ROOMNODE* roomHead): 방 목록 모두 삭제
*/
void deleteAllRoomNode(ROOMNODE* roomHead) {
	while (roomHead->link) {
		deleteRoomNode(roomHead, roomHead->link->data.roomNum);
	}
	return;
}

/*
void showRoomList(ROOMNODE* roomHead): 방 목록 새로고침
*/
void showRoomList(ROOMNODE* roomHead) {
	ROOMNODE* temp = roomHead;
	LVITEM LI;
	char tempStr[200];
	//리스트뷰 모두 지우고 새로고침
	ListView_DeleteAllItems(g_hRoomList);
	while (temp->link) {
		temp = temp->link;
		//LV에 방이름 정보 추가
		LI.mask = LVIF_TEXT;
		LI.iItem = 0;
		LI.iSubItem = 0;
		LI.pszText = temp->data.roomName;
		ListView_InsertItem(g_hRoomList, &LI);
		wsprintf(tempStr, "%d", temp->data.roomNum);
		//LV에 방번호 정보 추가
		ListView_SetItemText(g_hRoomList, 0, 1, tempStr);
	}
	return;
}

/*
void showUserList(USERNODE* userHead): 유저 목록 새로고침
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
ROOMNODE* findRoomNode(ROOMNODE* roomHead,int roomNum): 방번호가 roomNum인 방 찾아서 리턴
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