#include "common.h"
#include "chatRoom_server.h"
#include "mySocket.h"
#include "linkedList.h"
#include <commctrl.h>

//������ �ڵ�
HWND  g_hPortEdit, g_hMsgEdit, g_hRoomList, g_hUserList, g_hOpenBtn;

//������ ID
enum {
	ID_PORTEDIT = 1,
	ID_MSGEDIT,
	ID_OPENBTN = 101,
	ID_ROOMLIST = 201,
	ID_USERLIST
};

//��Ʈ��ȣ ����
unsigned short g_uPort;
//��Ĺapi�� ���Ϻ���
int nReturn;

HANDLE hSemaphore;
//������� ȯ�溯��
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
	//����Ʈ �ڽ� ����
	char strEdit[128];
	//����� ��Ĺ-user (Linked List)
	static USERNODE* userHead;
	//ä�ù� ����Ʈ-room (Linked List)
	static ROOMNODE* roomHead;
	//������ �Ű������� �ѱ� ������ ����
	static THREADDATA threadParam;
	//����Ʈ�� ȯ�� ����
	LVCOLUMN COL;
	LVITEM LI;
	//����Ʈ �ڽ� �� �޺� �ڽ� ���� ����
	char strList[128];
	DWORD ThreadID;
	static HANDLE hThread;
	static BOOL con = FALSE;

	switch (iMessage) {
	case WM_CREATE:
		InitCommonControls();
		g_hWndMain = hWnd;
		//��������
		hSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
		//head node �Ҵ�(user,room)
		userHead = createUserNode();
		roomHead = createRoomNode();

		///port �Է� ����Ʈ///
		g_hPortEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE, 170, 37, 150, 20, hWnd, (HMENU)ID_PORTEDIT, g_hInst, NULL);

		///����,���� ��ư///
		g_hOpenBtn = CreateWindow("button", "��������", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 335, 32, 80, 30, hWnd, (HMENU)ID_OPENBTN, g_hInst, NULL);

		////ä�ù� listbox ����////
		g_hRoomList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS, 20, 100, 250, 300, hWnd, (HMENU)ID_ROOMLIST, g_hInst, NULL);
		ListView_SetExtendedListViewStyle(g_hRoomList, LVS_EX_FULLROWSELECT);

		//ä�ù� ����Ʈ�信 ��� �߰�
		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 150;
		COL.pszText = (LPSTR)" ���̸� ";
		COL.iSubItem = 0;
		ListView_InsertColumn(g_hRoomList, 0, &COL);

		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 100;
		COL.pszText = (LPSTR)" �� ��ȣ ";
		COL.iSubItem = 1;
		ListView_InsertColumn(g_hRoomList, 1, &COL);
		///������ listbox ����///
		g_hUserList = CreateWindow("listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 350, 100, 200, 300, hWnd, (HMENU)ID_USERLIST, g_hInst, NULL);

		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_OPENBTN:
			//���� ����
			if (!con) {
				//PORT ����
				GetWindowText(g_hPortEdit, strEdit, 128);
				g_uPort = atoi(strEdit);

				//��Ĺ��� ȯ�� ����
				nReturn = WSAStartup(WORD(2.0), &g_env.wsadata);

				g_env.listensock = socket(AF_INET, SOCK_STREAM, 0);
				g_env.addr_server.sin_family = AF_INET;
				g_env.addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
				g_env.addr_server.sin_port = htons(g_uPort);
				nReturn = bind(g_env.listensock, (sockaddr*)&g_env.addr_server, sizeof(sockaddr));

				nReturn = listen(g_env.listensock, 1);
				//������ �Ű����� ����
				threadParam.sock = g_env.listensock;
				threadParam.userHead = userHead;
				threadParam.roomHead = roomHead;
				//accept ������ ����
				hThread = CreateThread(NULL, 0, AccThreadFunc, &threadParam, 0, &ThreadID);
				//�������� -> �����ݱ�, PORT ����Ʈ readOnly��
				SetWindowText(g_hOpenBtn, "�����ݱ�");
				SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)TRUE, 0);
				con = TRUE;
				MessageBox(hWnd, "�������� �Ϸ�", "���� ����", MB_OK);
			}
			//���� ����
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
				//�����ݱ� -> ��������, IP,PORT ����Ʈ readOnly ����
				SetWindowText(g_hOpenBtn, "��������");
				SendMessage(g_hPortEdit, EM_SETREADONLY, (WPARAM)FALSE, 0);
				con = FALSE;
				MessageBox(hWnd, "�������� ����", "���� ����", MB_OK);
			}
			break;
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		Rectangle(hdc, 112, 28, 425, 65);
		TextOut(hdc, 120, 38, "PORT", lstrlen("PORT"));
		TextOut(hdc, 30, 75, "ä�ù� ���", lstrlen("ä�ù� ���"));
		TextOut(hdc, 350, 75, "������ ���", lstrlen("������ ���"));
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



