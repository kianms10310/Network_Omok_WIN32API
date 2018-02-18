// NT.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "NT.h"

#define MAX_LOADSTRING 100

#define MAX_BOARD	19									// �ٵ��� 19 x 19
#define PORT 5412										// ��Ʈ��ȣ

// ����� ���� Windows �޽���
#define WM_CONNECT (WM_USER + 0)						
#define WM_RECEIVE (WM_USER + 1)
#define WM_ACCEPT (WM_USER + 2)



// ���� ����:
HMENU hMenu, hMenuReady, hMenuStart, hMenuCreate, hMenuConnect, hMenuDisconnect;	// �޴��ڵ�
HBITMAP hWndBit[3];																	// ��Ʈ�� �̹��� �ڵ�
HWND hWindow;																		// Window �ڵ� ����
TCHAR szIP_ADDRESS[20];																// IP �ּҸ� ������ �迭

static SOCKET client_sock;															// Ŭ���̾�Ʈ ����

typedef struct state{
	bool IsClient = FALSE;																// Ŭ���̾�Ʈ ����
	bool state_ServReady = FALSE;														// ������ �غ� ����
	bool state_ClntReady = FALSE;														// Ŭ���̾�Ʈ�� �غ� ����
	bool flag_GameStart = FALSE;														// ������ ����
	bool IsTurn = FALSE;																// �ڽ��� �������� ����
	bool flag_Start = FALSE;															// ���۹�ư�� �������� ����
	bool flag_End = FALSE;																// ������ ���� ����
}STATE;

STATE game_state;
/* ���� ������ ���� ���� */


int OMok[MAX_BOARD][MAX_BOARD];														// �ٵ��� �迭
int pX, pY;																			// ������ ��ǥ
int Turn;																			// ���� : 0�� �浹, 1�� �鵹 (0���� ����)
int Winner;																			// 0�� ��, 1�� ��

HINSTANCE hInst;																	// ���� �ν��Ͻ��Դϴ�.
TCHAR szTitle[MAX_LOADSTRING];														// ���� ǥ���� �ؽ�Ʈ�Դϴ�.
TCHAR szWindowClass[MAX_LOADSTRING];												// �⺻ â Ŭ���� �̸��Դϴ�.


// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);			// ��Ʈ�� �׸���
void DrawBg(HDC hdc);											// ��� �׸���(������)
void DrawOmok(HDC hdc);											// ���� �� �׸���(�ٵϾ�)
void AdPoint(int x, int y);										// ��ǥ ����
void Clear_OMOK();												// OMOK�� û��
bool put_OMok(int x, int y);									// ���� �ִ��� Ȯ��
int check();													// ��üũ
void ResultPrint(int x, int y);									
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK IPad(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ���⿡ �ڵ带 �Է��մϴ�.
	MSG msg;
	HACCEL hAccelTable;

	// ���� ���ڿ��� �ʱ�ȭ�մϴ�.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_NT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NT));

	// �⺻ �޽��� �����Դϴ�.
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NT));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_NT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   ����: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   ����:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

	hWnd = CreateWindow(szWindowClass, szTitle, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		0, 0, 620, 660, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����:  �� â�� �޽����� ó���մϴ�.
//
//  WM_COMMAND	- ���� ���α׷� �޴��� ó���մϴ�.
//  WM_PAINT	- �� â�� �׸��ϴ�.
//  WM_DESTROY	- ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	static int OptionValue = SO_SYNCHRONOUS_NONALERT;		// �˻� �ɼ� ������ ���� ���� ������
	WSADATA wsaData;										// ���� �ʱ�ȭ�� ���
	static SOCKET listen_sock;								// listen ����
	SOCKADDR_IN serveraddr, clientaddr, acceptaddr;			// ����, Ŭ���̾�Ʈ, accept �ּ� �� �����ϴ� ����ü
	int return_value;										// ��ȯ��
	char buffer[300];
	int pos;											
	int addrsize;											
	static RECT rtClient;
	static POINT pClient;
	int x, y;							

	switch (message)
	{
	case WM_CREATE:
		hWindow = hWnd;
		for (int i = 0; i < 3; i++)
			hWndBit[i] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BOARD + i));
		
		
		// �޴� ��Ȱ��ȭ, ��밡 �����ؾ� Ȱ��ȭ �Ǿ����.
		hMenu = GetMenu(hWnd);
		EnableMenuItem(hMenu,IDM_READY, MF_DISABLED);
		EnableMenuItem(hMenu, IDM_START, MF_DISABLED);
		EnableMenuItem(hMenu, IDM_DISCONNECT, MF_DISABLED);
		Clear_OMOK();						// �ٵ��� �ʱ�ȭ
		break;

		//client_sock FD_CONNECT �޽��� �߻���
	case WM_CONNECT:

		serveraddr.sin_family = AF_INET;						// TCP/IP
		serveraddr.sin_port = htons(PORT);						// ��Ʈ��ȣ
		serveraddr.sin_addr.s_addr = inet_addr(szIP_ADDRESS);	// �Է� ���� IP
		WSAAsyncSelect(client_sock, hWnd, NULL, NULL);			//Ŭ���̾�Ʈ ����

		connect(client_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

		// ���� üũ
		return_value = WSAGetLastError();

		// ������ ���� ���
		if (return_value == WSAEISCONN)
		{
			// ������ �޽��� ��� 
			// FD_ACCEPT : Ŭ���̾�Ʈ�� �����ϸ� ������ �޽����� �߻���Ų��.
			// FD_READ : ������ ������ �����ϸ� ������ �޽����� �߻���Ų��.
			// FD_WRITE : ������ �۽��� �����ϸ� ������ �޽����� �߻���Ų��.
			// FD_CLOSE : ��밡 ������ �����ϸ� ������ �޽����� �߻���Ų��.
			// FD_CONNECT : ������ �Ϸ�Ǹ� ������ �޽����� �߻���Ų��.
			// FD_OOB : OOB �����Ͱ� �����ϸ� ������ �޽����� �߻���Ų��.
			WSAAsyncSelect(client_sock, hWnd, WM_RECEIVE, FD_READ | FD_OOB | FD_CLOSE);
			MessageBox(hWnd, TEXT("Connect Success!!"), TEXT("Success"), MB_OK);

			// ���� �ʱ�ȭ
			game_state.IsClient = TRUE;
			game_state.IsTurn = FALSE;
			// �޴� ��Ȱ��ȭ �� Ȱ��ȭ
			EnableMenuItem(hMenu, IDM_READY, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_CROOM, MF_DISABLED);
			EnableMenuItem(hMenu, IDM_CONNECT, MF_DISABLED);
			EnableMenuItem(hMenu, IDM_DISCONNECT, MF_ENABLED);
		}

		// ���� �߻���
		else
		{
			MessageBox(hWnd, TEXT("Connect() Error!"), TEXT("Error"), MB_OK);
			closesocket(client_sock);
			WSACleanup();
		}
		break;

		//client_sock FD_READ | FD_OOB | FD_CLOSE �޽��� �߻���
		//client_sock FD_WRITE | FD_READ | FD_OOB | FD_CLOSE �޽��� �߻���
	case WM_RECEIVE:
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
			recv(client_sock, buffer, 128, 0);

			// ��밡 Ready�� ��.
			if (strcmp(buffer, "ready") == 0)
			{
				game_state.state_ClntReady = TRUE;
				MessageBox(hWnd, TEXT("Ready"), TEXT("Notice"), MB_OK);
			}

			// ������ ������ ������.
			else if (strcmp(buffer, "start") == 0)
			{
				game_state.flag_GameStart = TRUE;
				game_state.flag_Start = TRUE;
				game_state.flag_End = FALSE;
				InvalidateRect(hWnd, NULL, TRUE);
				MessageBox(hWnd, TEXT("Start Game!!"), TEXT("Notice"), MB_OK);
			}
			// ��� �ƴ϶�� ��ǥ�� �Ѱܿ�.
			else
			{
				// ������ ������ ���ڸ� pos ������ ����
				pos = atoi(buffer);
				y = pos % 1000;
				x = (pos - y) / 1000;
				ResultPrint(x, y);
			}

			break;
		// ���� �޽���
		case FD_CLOSE:
			MessageBox(hWnd, TEXT("DisConnect"), TEXT("Notice"), MB_ICONSTOP);

			// ���� CleanUP
			WSACleanup();

			// ���� �� ���� ���� �ʱ�ȭ
			game_state.IsClient = FALSE;
			//�޴� ����� �� Ȱ��ȭ
			EnableMenuItem(hMenu, IDM_START, MF_DISABLED);
			EnableMenuItem(hMenu, IDM_READY, MF_DISABLED);
			EnableMenuItem(hMenu, IDM_CROOM, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_CONNECT, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_DISCONNECT, MF_DISABLED);

			break;
		}
		break;

		// listen_sock FD_ACCEPT �޽��� �߻���
	case WM_ACCEPT:
		addrsize = sizeof(acceptaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&acceptaddr, &addrsize);
		WSAAsyncSelect(client_sock, hWnd, WM_RECEIVE, FD_WRITE | FD_READ | FD_OOB | FD_CLOSE);
		MessageBox(hWnd, TEXT("���� �Ǿ����ϴ�."), TEXT("�˸�"), MB_OK);
		EnableMenuItem(hMenu, IDM_READY, MF_ENABLED);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN:
		// ���� ���� ���
		if (game_state.flag_Start == TRUE && game_state.IsTurn == TRUE && game_state.flag_End == FALSE){
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			ResultPrint(x, y);
			_itoa(x * 1000 + y, buffer, 10);
			send(client_sock, buffer, 10, 0);
		}

		break;
	case WM_COMMAND:
		wmEvent = HIWORD(wParam);
		// �޴� ������ ���� �м��մϴ�.
		switch (LOWORD(wParam))
		{
			//�����ϱ�
		case IDM_START:
			// ������ Ŭ���̾�Ʈ ��� �غ� ���¶��
			if (game_state.state_ClntReady == TRUE && game_state.state_ServReady == TRUE)
			{
				// ������ ���
				if (game_state.IsClient == FALSE)
				{
					game_state.flag_GameStart = TRUE;
					game_state.flag_Start = TRUE;
					send(client_sock, "start", 10, 0);
					game_state.IsTurn = TRUE;						//������ ���� ������ �����Ѵ�
					game_state.flag_End = FALSE;
					InvalidateRect(hWnd, NULL, TRUE);

					EnableMenuItem(hMenu, IDM_READY, MF_DISABLED);
					EnableMenuItem(hMenu, IDM_START, MF_DISABLED);
				}
			}
			break;

			//�غ��ϱ�
		case IDM_READY:
			if (game_state.IsClient == TRUE)
			{					//Ŭ���̾�Ʈ ���
				send(client_sock, "ready", 10, 0);	//�������� �غ� �Ǿ����� �˸�
			}
			else
			{										//�������
				game_state.state_ServReady = TRUE;
				EnableMenuItem(hMenu, IDM_START, MF_ENABLED);
			}
			EnableMenuItem(hMenu, IDM_READY, MF_DISABLED);
			break;

			//�������
		case IDM_DISCONNECT:
			MessageBox(hWnd, TEXT("Disconnect"), TEXT("Notice"), MB_ICONSTOP);
			closesocket(client_sock);
			closesocket(listen_sock);
			WSACleanup();
			game_state.IsClient = FALSE;
			EnableMenuItem(hMenu, IDM_START, MF_DISABLED);
			EnableMenuItem(hMenu, IDM_READY, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_CROOM, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_CONNECT, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_DISCONNECT, MF_ENABLED);
			break;

			//�游���
		case IDM_CROOM:
			//���� �ʱ�ȭ
			WSAStartup(MAKEWORD(2, 2), &wsaData);

			//���� �ɼ� ����
			setsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (char*)&OptionValue, sizeof(int));

			//���� ����(TCP)
			listen_sock = socket(AF_INET, SOCK_STREAM, 0);

			//�ּ�ü�� ����
			clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
			clientaddr.sin_family = AF_INET;
			clientaddr.sin_port = htons(PORT);

			//���ε�
			return_value = bind(listen_sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));
			if (return_value == SOCKET_ERROR)
			{
				MessageBox(hWnd, TEXT("Bind() Error!"), TEXT("Error"), MB_OK);
				PostQuitMessage(0);			//������ ����(�ӽ�)-���߿� �ٲ�
			}
			//������ �޽��� ���(LISTEN ���Ͽ� ���� ��û�� ������ WM_ACCEPT �߻�)
			// FD_ACCEPT : Ŭ���̾�Ʈ�� �����ϸ� ������ �޽����� �߻���Ų��.
			WSAAsyncSelect(listen_sock, hWnd, WM_ACCEPT, FD_ACCEPT);

			//1�� �����Ѵ�.
			// 1:1 ����
			listen(listen_sock, 1);

			//�޽��� �ڽ�!
			MessageBox(hWnd, TEXT("Create ROOM Success!!"), TEXT("Success"), MB_OK);

			//�� ����� ��ư ��Ȱ��ȭ(���� ��������Ƿ�)
			EnableMenuItem(hMenu, IDM_CROOM, MF_DISABLED);

			//�����ϱ� ��ư ��Ȱ��ȭ(������ �ǹǷ� Ŭ���̾�Ʈ�� ��ư�� ���ش�)
			EnableMenuItem(hMenu, IDM_CONNECT, MF_DISABLED);

			break;
		
			//�����ϱ�
		case IDM_CONNECT:
			if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, IPad))
			{

				//���� �ʱ�ȭ
				WSAStartup(MAKEWORD(2, 2), &wsaData);

				setsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (char*)&OptionValue, sizeof(int));

				// Ŭ���̾�Ʈ ���� ����
				client_sock = socket(AF_INET, SOCK_STREAM, 0);

				// �ּ�ü�� ����
				serveraddr.sin_addr.s_addr = inet_addr(szIP_ADDRESS);		// ���̾�α׷� �Է¹��� ip�ּ�
				serveraddr.sin_family = AF_INET;						// TCP/IP
				serveraddr.sin_port = htons(PORT);						// ��Ʈ��ȣ

				// ������ �޽��� ���
				// FD_CONNECT : ������ �Ϸ�Ǹ� ������ �޽����� �߻���Ų��.
				WSAAsyncSelect(client_sock, hWnd, WM_CONNECT, FD_CONNECT);

				// �����û
				connect(client_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
			}
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DrawBg(hdc);
		// TODO: ���⿡ �׸��� �ڵ带 �߰��մϴ�.

		if (game_state.flag_End == FALSE)
			DrawOmok(hdc);		// �ٵϵ� �׸���
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK IPad(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{


	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == ID_OK){
			GetDlgItemText(hDlg, IDC_EDIT1, szIP_ADDRESS, 20);

			EndDialog(hDlg, ID_OK);
			return TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

void DrawOmok(HDC hdc)
{
	// ���� �� �� ��
	if (game_state.flag_Start)
	{

		for (int i = 0; i < MAX_BOARD; i++)
		{
			for (int j = 0; j<MAX_BOARD; j++)
			{
				// �浹
				if (OMok[i][j] == 0)
					DrawBitmap(hdc, j * 32 + 3, i * 32 + 3, hWndBit[1]);
				// �鵹
				else if (OMok[i][j] == 1)
					DrawBitmap(hdc, j * 32 + 3, i * 32 + 3, hWndBit[2]);
			}
		}
	}
}

void DrawBg(HDC hdc)
{
	DrawBitmap(hdc, 0, 0, hWndBit[0]);
}

void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
	HDC MemDC;
	HBITMAP OldBitmap;
	int bitx, bity;
	BITMAP bit;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit, sizeof(BITMAP), &bit);
	bitx = bit.bmWidth;
	bity = bit.bmHeight;

	BitBlt(hdc, x, y, bitx, bity, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}

void AdPoint(int x, int y)
{
	pX = x;
	pY = y;

	pX = pX / 32;
	pX = pX * 32 + 3;

	pY = pY / 32;
	pY = pY * 32 + 3;
}

void Clear_OMOK()
{
	for (int i = 0; i<MAX_BOARD; i++){
		for (int j = 0; j<MAX_BOARD; j++){
			OMok[i][j] = -1;
		}
	}
	game_state.flag_Start = FALSE;							// ���� X
	game_state.flag_End = TRUE;							// ���� O
	game_state.state_ServReady = FALSE;					// ���� �غ� X
	game_state.state_ClntReady = FALSE;					// Ŭ���̾�Ʈ �غ� X
	game_state.flag_GameStart = FALSE;						// ���� ���� X
	game_state.IsTurn = FALSE;								// �ڽ��� ���� X
	Turn = 0;									// ���� ��
	

}


bool put_OMok(int x, int y)
{

	// ��ǥ�� �迭�� ��ȯ
	x = x / 32;
	y = y / 32;

	if (OMok[y][x] != -1){
		return FALSE;	// �̹� ���� �ִٸ� false
	}

	OMok[y][x] = Turn;	// ���� ��


	return true;
}

int check()
{
	int iCheck;

	for (int i = 0; i<MAX_BOARD; i++){
		for (int j = 0; j<MAX_BOARD; j++){
			iCheck = 0;
			// ������ ���񿩺� �˻�
			for (int k = 0; (j + 5 < MAX_BOARD) && (k < 5); k++)
			{		
				if ((Turn) == (OMok[i][j + k]))
					iCheck++;
			}
			if (iCheck == 5)
				return Turn;
			
			iCheck = 0;

			// (������)�밢���� ���񿩺� �˻�
			for (int k = 0; (i + 5 < MAX_BOARD) && (j + 5 < MAX_BOARD) && (k < 5); k++)
			{
				if ((Turn) == (OMok[i + k][j + k]))
					iCheck++;
			}
			if (iCheck == 5)
				return Turn;
			

			iCheck = 0;
			// (������)�밢���� ���񿩺� �˻�
			for (int k = 0; (i + 5 < MAX_BOARD) && (j - 5 >= 0) && (k < 5); k++)
			{
				if ((Turn) == (OMok[i + k][j - k]))
					iCheck++;
			}
			if (iCheck == 5)
				return Turn;
			
			iCheck = 0;

			// ������ ���񿩺� �˻�
			for (int k = 0; (i + 5 < MAX_BOARD) && (k < 5); k++)
			{
				if ((Turn) == (OMok[i + k][j]))
					iCheck++;
			}
			if (iCheck == 5)
				return Turn;
		}
	}
	return -1;
}

void ResultPrint(int x, int y)
{
	int iCheck;		// �������� ����
	int OMok_check;	// ���� ���� �ִ��� ����
	RECT rt;

	if (x > 0 && x < 600 && y > 0 && y < 600)
	{
		AdPoint(x, y);
		OMok_check = put_OMok(x, y);

		if (OMok_check == TRUE)
		{	
			// ���� ���� ���
			SetRect(&rt, pX, pY, pX + 31, pY + 31);
			InvalidateRect(hWindow, &rt, FALSE);

			// ������ ���θ� ����
			iCheck = check();

			switch (iCheck)
			{
			case -1:
				break;
			case 0:
				MessageBox(hWindow, TEXT("Black WIN!~"), TEXT("Notice"), MB_OK);
				EnableMenuItem(hMenu, IDM_START, MF_DISABLED);
				EnableMenuItem(hMenu, IDM_READY, MF_ENABLED);
				EnableMenuItem(hMenu, IDM_CROOM, MF_DISABLED);
				EnableMenuItem(hMenu, IDM_CONNECT, MF_DISABLED);
				EnableMenuItem(hMenu, IDM_DISCONNECT, MF_ENABLED);

				Clear_OMOK();
				break;
			case 1:
				MessageBox(hWindow, TEXT("White WIN!~"), TEXT("Notice"), MB_OK);
				EnableMenuItem(hMenu, IDM_START, MF_DISABLED);
				EnableMenuItem(hMenu, IDM_READY, MF_ENABLED);
				EnableMenuItem(hMenu, IDM_CROOM, MF_DISABLED);
				EnableMenuItem(hMenu, IDM_CONNECT, MF_DISABLED);
				EnableMenuItem(hMenu, IDM_DISCONNECT, MF_ENABLED);
				Clear_OMOK();
				break;
			}

			if (iCheck == -1)
			{
				if (Turn == 0)
					Turn = 1;
				else
					Turn = 0;
				if (game_state.IsTurn == TRUE)
					game_state.IsTurn = FALSE;
				else
					game_state.IsTurn = TRUE;
			}
		}
	}
}