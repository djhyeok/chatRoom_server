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
void cutMsg(char* msg,char* buffer): buffer에서 메시지 잘라서 msg에 담음
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
DWORD WINAPI AccThreadFunc(LPVOID Param): accept()가 동작하면 recv Thread를 생성하는 accept Thread함수
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
			SendMessage(g_hRoomList, LB_ADDSTRING, 0, (LPARAM)"클라이언트가 접속하였습니다.");
			//usernode 추가,recv 스레드 생성 후 userList 새로고침 후 보여줌
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
DWORD WINAPI RecvThreadFunc(LPVOID Param): 리시브 스레드 함수
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
		if (nReturn > 0) {				//데이터 전송이 왔을 경우
			//유저등록 요청			"R유저이름"
			if (g_env.buf[0] == 'R') {
				//해당 소캣과 같은 노드를 찾아 유저이름 설정
				cutMsg(userNode->data.userName, g_env.buf);
				showUserList(P.userHead);
			}
			//채팅방리스트 요청		"I"
			else if (g_env.buf[0] == 'I') {
				tempRoom = P.roomHead;
				//요청한 client에게 모든 방의 정보를 "c방번호|방이름" send 해서 client에서 추가할 수 있게 함
				while (tempRoom->link) {
					tempRoom = tempRoom->link;
					wsprintf(msgTemp, "c%d|%s", tempRoom->data.roomNum, tempRoom->data.roomName);
					nReturn = send(P.sock, msgTemp, sizeof(msgTemp), 0);
				}
			}
			//채팅방 개설요청		"C채팅방이름"
			else if (g_env.buf[0] == 'C') {
				//해당 소캣과 같은 유저노드를 찾아 방이름 파싱 후 방 개설
				cutMsg(strTemp, g_env.buf);
				num = addRoomNode(P.roomHead, strTemp, userNode->data);
				//생성된 방 정보(방 제목,방 번호) 채팅방 리스트뷰에 뿌림
				LI.mask = LVIF_TEXT;
				LI.iItem = 0;
				LI.iSubItem = 0;
				LI.pszText = strTemp;
				ListView_InsertItem(g_hRoomList, &LI);
				wsprintf(strTemp, "%d", num);
				ListView_SetItemText(g_hRoomList, 0, 1, strTemp);
				//생성된 방 정보를 모든 클라이언트에 보냄
				roomNode = findRoomNode(P.roomHead, num);
				tempUser = P.userHead;
				while (tempUser->link) {
					tempUser = tempUser->link;
					//모든 클라이언트에 c방번호|방이름 send
					wsprintf(msgTemp, "c%s|%s", strTemp, roomNode->data.roomName);
					nReturn = send(tempUser->data.clientsock, msgTemp, sizeof(msgTemp), 0);
					//방생성 요청을 보낸 사용자가 해당 방 참여할 수 있게 send
					if (tempUser == userNode) {
						//방 참여 메시지 p방번호 send
						wsprintf(msgTemp, "p%s", strTemp);
						nReturn = send(userNode->data.clientsock, msgTemp, sizeof(msgTemp), 0);
					}
				}
			}
			//채팅방 참여요청		"P방번호"
			else if (g_env.buf[0] == 'P') {
				//strTemp에 방번호 담음
				cutMsg(strTemp, g_env.buf);
				//해당 방번호와 일치하는 방 roomNode에 복사하고 해당 방에 userHead에 참여유저 추가
				roomNode = findRoomNode(P.roomHead, atoi(strTemp));
				addUserNode(roomNode->data.roomUserHead, userNode->data);
				//방 참여 메시지 p방번호 send
				wsprintf(msgTemp, "p%s", strTemp);
				nReturn = send(P.sock, msgTemp, sizeof(msgTemp), 0);
			}
			//메시지 전달요청		"M메시지"
			else if (g_env.buf[0] == 'M') {
				cutMsg(strTemp, g_env.buf);
				//tempUser에 현재 방 참여자 LinkedList 헤드 복사
				tempUser = roomNode->data.roomUserHead;
				//메시지 가공     "M보낸사람:메시지"
				wsprintf(msgTemp, "m%s:%s", userNode->data.userName, strTemp);
				while (tempUser->link) {
					tempUser = tempUser->link;
					//본인에게 보낼 경우
					if (tempUser->data.clientsock == userNode->data.clientsock) {
						wsprintf(strTemp2, "m(나):%s", strTemp);
						nReturn = send(tempUser->data.clientsock, strTemp2, sizeof(strTemp2), 0);
					}
					//본인이 아닌 경우
					else {
						nReturn = send(tempUser->data.clientsock, msgTemp, sizeof(msgTemp), 0);
					}
				}
			}
			//파일 전송 요청 "F파일크기"
			else if (g_env.buf[0] == 'F') {
				//파일 전송 은 한 클라이언트가 끝날때까지 다른 클라이언트는 대기
				WaitForSingleObject(hSemaphore, INFINITE);
				cutMsg(strTemp, g_env.buf);
				imgSize = atoi(strTemp);
				lstrcpy(msgTemp, g_env.buf);
				msgTemp[0] = 'f';
				////현재 방에 참여자들에게 f파일크기 send
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
			//채팅방 나가기요청		"E방번호"
			else if (g_env.buf[0] == 'E') {
				cutMsg(strTemp, g_env.buf);
				num = atoi(strTemp);
				//방번호에 해당하는 방 노드 탐색
				roomNode = findRoomNode(P.roomHead, num);
				//방 참여 인원 LinkedList에서 유저 삭제
				deleteUserNode(roomNode->data.roomUserHead, P.sock);
				//나간 유저가 마지막 유저였을 경우
				if (roomNode->data.roomUserHead->link == NULL) {
					//채팅방 나가기요청,삭제
					wsprintf(msgTemp, "e%d", roomNode->data.roomNum);
					//해당 방번호를 가진 방 삭제
					deleteRoomNode(P.roomHead, num);
					tempUser = P.userHead;
					//삭제된 방을 다른 사용자에서도 삭제할수 있게 send
					while (tempUser->link) {
						tempUser = tempUser->link;
						nReturn = send(tempUser->data.clientsock, msgTemp, sizeof(msgTemp), 0);
					}
					//채팅방 노드 삭제후 채팅방 리스트 새로고침
					showRoomList(P.roomHead);
				}
				//나간 유저가 마지막이 아닌경우
				else {
					//채팅방 나가기 요청 완료 "e"
					nReturn = send(P.sock, "e", sizeof("e"), 0);
				}
			}
		}
		else if (nReturn == 0) {		//클라이언트 정상 종료(클라이언트에서 shutdown)
			deleteUserNode(P.userHead, P.sock);
			showUserList(P.userHead);
			shutdown(P.sock, 1);
			closesocket(P.sock);
			ExitThread(0);
		}
		else {							//비정상 종료
			MessageBox(g_hWndMain, "수신에러", "수신에러", MB_OK);
			break;
		}
	}
	return 0;
}