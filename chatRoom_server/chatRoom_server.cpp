#include "common.h"
#include "chatRoom_server.h"
#include "mySocket.h"
#include "linkedList.h"
#include <commctrl.h>

//윈도우 핸들
HWND  g_hPortEdit, g_hMsgEdit, g_hRoomList, g_hUserList, g_hOpenBtn;

//윈도우 ID
enum {
	ID_PORTEDIT = 1,
	ID_MSGEDIT,
	ID_OPENBTN = 101,
	ID_ROOMLIST = 201,
	ID_USERLIST
};

//포트번호 변수
unsigned short g_uPort;
//소캣api의 리턴변수
int nReturn;

HANDLE hSemaphore;
//소켓통신 환경변수
ENV g_env;
HWND g_hWndMain;
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("ChatRoom");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_CAPTION | WS_SYSMENU, 0, 0, 600, 500, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	int i;
	HDC hdc;
	PAINTSTRUCT ps;
	//에디트 박스 변수
	char strEdit[128];
	//연결된 소캣-user (Linked List)
	static USERNODE* userHead;
	//채팅방 리스트-room (Linked List)
	static ROOMNODE* roomHead;
	//스레드 매개변수로 넘길 데이터 변수
	static THREADDATA threadParam;
	//리스트뷰 환경 변수
	LVCOLUMN COL;
	LVITEM LI;
	//리스트 박스 및 콤보 박스 공통 변수
	char strList[128];
	DWORD ThreadID;
	static HANDLE hThread;
	static BOOL con = FALSE;

	switch (iMessage) {
	case WM_CREATE:
		InitCommonControls();
		g_hWndMain = hWnd;
		//세마포어
		hSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
		//head node 할당(user,room)
		userHead = createUserNode();
		roomHead = createRoomNode();

		///port 입력 에디트///
		g_hPortEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE, 170, 37, 150, 20, hWnd, (HMENU)ID_PORTEDIT, g_hInst, NULL);

		///접속,해제 버튼///
		g_hOpenBtn = CreateWindow("button", "서버열기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 335, 32, 80, 30, hWnd, (HMENU)ID_OPENBTN, g_hInst, NULL);

		////채팅방 listbox 생성////
		g_hRoomList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS, 20, 100, 250, 300, hWnd, (HMENU)ID_ROOMLIST, g_hInst, NULL);
		ListView_SetExtendedListViewStyle(g_hRoomList, LVS_EX_FULLROWSELECT);

		//채팅방 리스트뷰에 헤더 추가
		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 150;
		COL.pszText = (LPSTR)" 방이름 ";
		COL.iSubItem = 0;
		ListView_InsertColumn(g_hRoomList, 0, &COL);

		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 100;
		COL.pszText = (LPSTR)" 방 번호 ";
		COL.iSubItem = 1;
		ListView_InsertColumn(g_hRoomList, 1, &COL);
		///접속자 listbox 생성///
		g_hUserList = CreateWindow("listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 350, 100, 200, 300, hWnd, (HMENU)ID_USERLIST, g_hInst, NULL);

		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_OPENBTN:
			//서버 연결
			if (!con) {
				//PORT 설정
				GetWindowText(g_hPortEdit, strEdit, 128);
				g_uPort = atoi(strEdit);

				//소캣통신 환경 설정
				nReturn = WSAStartup(WORD(2.0), &g_env.wsadata);

				g_env.listensock = socket(AF_INET, SOCK_STREAM, 0);
				g_env.addr_server.sin_family = AF_INET;
				g_env.addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
				g_env.addr_server.sin_port = htons(g_uPort);
				nReturn = bind(g_env.listensock, (sockaddr*)&g_env.addr_server, sizeof(sockaddr));

				nReturn = listen(g_env.listensock, 1);
				//스레드 매개변수 세팅
				threadParam.sock = g_env.listensock;
				threadParam.userHead = userHead;
				threadParam.roomHead = roomHead;
				//accept 스레드 생성
				hThread = CreateThread(NULL, 0, AccThreadFunc, &threadParam, 0, &ThreadID);
				//서버열기 -> 서버닫기, PORT 에디트 readOnly로
				SetWindowText(g_hOpenBtn, "서버닫기");
				SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)TRUE, 0);
				con = TRUE;
				MessageBox(hWnd, "서버연결 완료", "연결 성공", MB_OK);
			}
			//연결 해제
			else {
				USERNODE* tempUser = userHead;
				while (tempUser->link) {
					tempUser = tempUser->link;
					shutdown(tempUser->data.clientsock, 0);
					closesocket(tempUser->data.clientsock);
				}
				deleteAllUserNode(userHead);
				deleteAllRoomNode(roomHead);
				SetWindowText(g_hPortEdit, "");
				SendMessage(g_hUserList, LB_RESETCONTENT, 0, 0);
				ListView_DeleteAllItems(g_hRoomList);
				closesocket(g_env.listensock);
				WSACleanup();
				//서버닫기 -> 서버열기, IP,PORT 에디트 readOnly 해제
				SetWindowText(g_hOpenBtn, "서버열기");
				SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
				con = FALSE;
				MessageBox(hWnd, "서버연결 해제", "접속 해제", MB_OK);
			}
			break;
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		Rectangle(hdc, 112, 28, 425, 65);
		TextOut(hdc, 120, 38, "PORT", lstrlen("PORT"));
		TextOut(hdc, 30, 75, "채팅방 목록", lstrlen("채팅방 목록"));
		TextOut(hdc, 350, 75, "접속자 목록", lstrlen("접속자 목록"));
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		if (con) {
			deleteAllUserNode(userHead);
			WSACleanup();
		}
		free(userHead);
		free(roomHead);
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}



