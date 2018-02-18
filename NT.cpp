// NT.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "NT.h"

#define MAX_LOADSTRING 100

#define MAX_BOARD	19									// 바둑판 19 x 19
#define PORT 5412										// 포트번호

// 사용자 정의 Windows 메시지
#define WM_CONNECT (WM_USER + 0)						
#define WM_RECEIVE (WM_USER + 1)
#define WM_ACCEPT (WM_USER + 2)



// 전역 변수:
HMENU hMenu, hMenuReady, hMenuStart, hMenuCreate, hMenuConnect, hMenuDisconnect;	// 메뉴핸들
HBITMAP hWndBit[3];																	// 비트맵 이미지 핸들
HWND hWindow;																		// Window 핸들 저장
TCHAR szIP_ADDRESS[20];																// IP 주소를 저장할 배열

static SOCKET client_sock;															// 클라이언트 소켓

typedef struct state{
	bool IsClient = FALSE;																// 클라이언트 여부
	bool state_ServReady = FALSE;														// 서버의 준비 상태
	bool state_ClntReady = FALSE;														// 클라이언트의 준비 상태
	bool flag_GameStart = FALSE;														// 게임중 여부
	bool IsTurn = FALSE;																// 자신의 차례인지 여부
	bool flag_Start = FALSE;															// 시작버튼을 눌렀는지 여부
	bool flag_End = FALSE;																// 게임의 끝의 여부
}STATE;

STATE game_state;
/* 게임 진행을 위한 변수 */


int OMok[MAX_BOARD][MAX_BOARD];														// 바둑판 배열
int pX, pY;																			// 보정된 좌표
int Turn;																			// 차례 : 0은 흑돌, 1은 백돌 (0부터 시작)
int Winner;																			// 0은 흑, 1은 백

HINSTANCE hInst;																	// 현재 인스턴스입니다.
TCHAR szTitle[MAX_LOADSTRING];														// 제목 표시줄 텍스트입니다.
TCHAR szWindowClass[MAX_LOADSTRING];												// 기본 창 클래스 이름입니다.


// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);			// 비트맵 그리기
void DrawBg(HDC hdc);											// 배경 그리기(오목판)
void DrawOmok(HDC hdc);											// 오목 알 그리기(바둑알)
void AdPoint(int x, int y);										// 좌표 보정
void Clear_OMOK();												// OMOK판 청소
bool put_OMok(int x, int y);									// 알이 있는지 확인
int check();													// 승체크
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

	// TODO: 여기에 코드를 입력합니다.
	MSG msg;
	HACCEL hAccelTable;

	// 전역 문자열을 초기화합니다.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_NT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NT));

	// 기본 메시지 루프입니다.
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
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
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
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

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
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND	- 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT	- 주 창을 그립니다.
//  WM_DESTROY	- 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	static int OptionValue = SO_SYNCHRONOUS_NONALERT;		// 검색 옵션 저장을 위한 버퍼 포인팅
	WSADATA wsaData;										// 윈속 초기화시 사용
	static SOCKET listen_sock;								// listen 소켓
	SOCKADDR_IN serveraddr, clientaddr, acceptaddr;			// 서버, 클라이언트, accept 주소 값 저장하는 구조체
	int return_value;										// 반환값
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
		
		
		// 메뉴 비활성화, 상대가 연결해야 활성화 되어야함.
		hMenu = GetMenu(hWnd);
		EnableMenuItem(hMenu,IDM_READY, MF_DISABLED);
		EnableMenuItem(hMenu, IDM_START, MF_DISABLED);
		EnableMenuItem(hMenu, IDM_DISCONNECT, MF_DISABLED);
		Clear_OMOK();						// 바둑판 초기화
		break;

		//client_sock FD_CONNECT 메시지 발생시
	case WM_CONNECT:

		serveraddr.sin_family = AF_INET;						// TCP/IP
		serveraddr.sin_port = htons(PORT);						// 포트번호
		serveraddr.sin_addr.s_addr = inet_addr(szIP_ADDRESS);	// 입력 받은 IP
		WSAAsyncSelect(client_sock, hWnd, NULL, NULL);			//클라이언트 소켓

		connect(client_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

		// 에러 체크
		return_value = WSAGetLastError();

		// 에러가 없을 경우
		if (return_value == WSAEISCONN)
		{
			// 윈도우 메시지 등록 
			// FD_ACCEPT : 클라이언트가 접속하면 윈도우 메시지를 발생시킨다.
			// FD_READ : 데이터 수신이 가능하면 윈도우 메시지를 발생시킨다.
			// FD_WRITE : 데이터 송신이 가능하면 윈도우 메시지를 발생시킨다.
			// FD_CLOSE : 상대가 접속을 종료하면 윈도우 메시지를 발생시킨다.
			// FD_CONNECT : 접속이 완료되면 윈도우 메시지를 발생시킨다.
			// FD_OOB : OOB 데이터가 도착하면 윈도우 메시지를 발생시킨다.
			WSAAsyncSelect(client_sock, hWnd, WM_RECEIVE, FD_READ | FD_OOB | FD_CLOSE);
			MessageBox(hWnd, TEXT("Connect Success!!"), TEXT("Success"), MB_OK);

			// 변수 초기화
			game_state.IsClient = TRUE;
			game_state.IsTurn = FALSE;
			// 메뉴 비활성화 및 활성화
			EnableMenuItem(hMenu, IDM_READY, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_CROOM, MF_DISABLED);
			EnableMenuItem(hMenu, IDM_CONNECT, MF_DISABLED);
			EnableMenuItem(hMenu, IDM_DISCONNECT, MF_ENABLED);
		}

		// 에러 발생시
		else
		{
			MessageBox(hWnd, TEXT("Connect() Error!"), TEXT("Error"), MB_OK);
			closesocket(client_sock);
			WSACleanup();
		}
		break;

		//client_sock FD_READ | FD_OOB | FD_CLOSE 메시지 발생시
		//client_sock FD_WRITE | FD_READ | FD_OOB | FD_CLOSE 메시지 발생시
	case WM_RECEIVE:
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
			recv(client_sock, buffer, 128, 0);

			// 상대가 Ready를 함.
			if (strcmp(buffer, "ready") == 0)
			{
				game_state.state_ClntReady = TRUE;
				MessageBox(hWnd, TEXT("Ready"), TEXT("Notice"), MB_OK);
			}

			// 서버가 게임을 시작함.
			else if (strcmp(buffer, "start") == 0)
			{
				game_state.flag_GameStart = TRUE;
				game_state.flag_Start = TRUE;
				game_state.flag_End = FALSE;
				InvalidateRect(hWnd, NULL, TRUE);
				MessageBox(hWnd, TEXT("Start Game!!"), TEXT("Notice"), MB_OK);
			}
			// 모두 아니라면 좌표를 넘겨옴.
			else
			{
				// 상대방이 선택한 문자를 pos 변수에 저장
				pos = atoi(buffer);
				y = pos % 1000;
				x = (pos - y) / 1000;
				ResultPrint(x, y);
			}

			break;
		// 종료 메시지
		case FD_CLOSE:
			MessageBox(hWnd, TEXT("DisConnect"), TEXT("Notice"), MB_ICONSTOP);

			// 윈속 CleanUP
			WSACleanup();

			// 변수 및 각종 변수 초기화
			game_state.IsClient = FALSE;
			//메뉴 숨기기 및 활성화
			EnableMenuItem(hMenu, IDM_START, MF_DISABLED);
			EnableMenuItem(hMenu, IDM_READY, MF_DISABLED);
			EnableMenuItem(hMenu, IDM_CROOM, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_CONNECT, MF_ENABLED);
			EnableMenuItem(hMenu, IDM_DISCONNECT, MF_DISABLED);

			break;
		}
		break;

		// listen_sock FD_ACCEPT 메시지 발생시
	case WM_ACCEPT:
		addrsize = sizeof(acceptaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&acceptaddr, &addrsize);
		WSAAsyncSelect(client_sock, hWnd, WM_RECEIVE, FD_WRITE | FD_READ | FD_OOB | FD_CLOSE);
		MessageBox(hWnd, TEXT("연결 되었습니다."), TEXT("알림"), MB_OK);
		EnableMenuItem(hMenu, IDM_READY, MF_ENABLED);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN:
		// 게임 중일 경우
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
		// 메뉴 선택을 구문 분석합니다.
		switch (LOWORD(wParam))
		{
			//시작하기
		case IDM_START:
			// 서버와 클라이언트 모두 준비 상태라면
			if (game_state.state_ClntReady == TRUE && game_state.state_ServReady == TRUE)
			{
				// 서버일 경우
				if (game_state.IsClient == FALSE)
				{
					game_state.flag_GameStart = TRUE;
					game_state.flag_Start = TRUE;
					send(client_sock, "start", 10, 0);
					game_state.IsTurn = TRUE;						//서버가 먼저 게임을 시작한다
					game_state.flag_End = FALSE;
					InvalidateRect(hWnd, NULL, TRUE);

					EnableMenuItem(hMenu, IDM_READY, MF_DISABLED);
					EnableMenuItem(hMenu, IDM_START, MF_DISABLED);
				}
			}
			break;

			//준비하기
		case IDM_READY:
			if (game_state.IsClient == TRUE)
			{					//클라이언트 라면
				send(client_sock, "ready", 10, 0);	//서버에게 준비가 되었음을 알림
			}
			else
			{										//서버라면
				game_state.state_ServReady = TRUE;
				EnableMenuItem(hMenu, IDM_START, MF_ENABLED);
			}
			EnableMenuItem(hMenu, IDM_READY, MF_DISABLED);
			break;

			//연결끊기
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

			//방만들기
		case IDM_CROOM:
			//윈속 초기화
			WSAStartup(MAKEWORD(2, 2), &wsaData);

			//소켓 옵션 변경
			setsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (char*)&OptionValue, sizeof(int));

			//소켓 생성(TCP)
			listen_sock = socket(AF_INET, SOCK_STREAM, 0);

			//주소체계 세팅
			clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
			clientaddr.sin_family = AF_INET;
			clientaddr.sin_port = htons(PORT);

			//바인딩
			return_value = bind(listen_sock, (SOCKADDR*)&clientaddr, sizeof(clientaddr));
			if (return_value == SOCKET_ERROR)
			{
				MessageBox(hWnd, TEXT("Bind() Error!"), TEXT("Error"), MB_OK);
				PostQuitMessage(0);			//에러시 종료(임시)-나중에 바꿈
			}
			//윈도우 메시지 등록(LISTEN 소켓에 연결 요청이 들어오면 WM_ACCEPT 발생)
			// FD_ACCEPT : 클라이언트가 접속하면 윈도우 메시지를 발생시킨다.
			WSAAsyncSelect(listen_sock, hWnd, WM_ACCEPT, FD_ACCEPT);

			//1명만 수용한다.
			// 1:1 게임
			listen(listen_sock, 1);

			//메시지 박스!
			MessageBox(hWnd, TEXT("Create ROOM Success!!"), TEXT("Success"), MB_OK);

			//방 만들기 버튼 비활성화(방을 만들었으므로)
			EnableMenuItem(hMenu, IDM_CROOM, MF_DISABLED);

			//연결하기 버튼 비활성화(서버가 되므로 클라이언트용 버튼은 없앤다)
			EnableMenuItem(hMenu, IDM_CONNECT, MF_DISABLED);

			break;
		
			//연결하기
		case IDM_CONNECT:
			if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, IPad))
			{

				//윈속 초기화
				WSAStartup(MAKEWORD(2, 2), &wsaData);

				setsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (char*)&OptionValue, sizeof(int));

				// 클라이언트 소켓 생성
				client_sock = socket(AF_INET, SOCK_STREAM, 0);

				// 주소체계 설정
				serveraddr.sin_addr.s_addr = inet_addr(szIP_ADDRESS);		// 다이얼로그로 입력받은 ip주소
				serveraddr.sin_family = AF_INET;						// TCP/IP
				serveraddr.sin_port = htons(PORT);						// 포트번호

				// 윈도우 메시지 등록
				// FD_CONNECT : 접속이 완료되면 윈도우 메시지를 발생시킨다.
				WSAAsyncSelect(client_sock, hWnd, WM_CONNECT, FD_CONNECT);

				// 연결요청
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
		// TODO: 여기에 그리기 코드를 추가합니다.

		if (game_state.flag_End == FALSE)
			DrawOmok(hdc);		// 바둑돌 그리기
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

// 정보 대화 상자의 메시지 처리기입니다.
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
	// 게임 중 일 때
	if (game_state.flag_Start)
	{

		for (int i = 0; i < MAX_BOARD; i++)
		{
			for (int j = 0; j<MAX_BOARD; j++)
			{
				// 흑돌
				if (OMok[i][j] == 0)
					DrawBitmap(hdc, j * 32 + 3, i * 32 + 3, hWndBit[1]);
				// 백돌
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
	game_state.flag_Start = FALSE;							// 시작 X
	game_state.flag_End = TRUE;							// 끝남 O
	game_state.state_ServReady = FALSE;					// 서버 준비 X
	game_state.state_ClntReady = FALSE;					// 클라이언트 준비 X
	game_state.flag_GameStart = FALSE;						// 게임 시작 X
	game_state.IsTurn = FALSE;								// 자신의 차례 X
	Turn = 0;									// 흑이 선
	

}


bool put_OMok(int x, int y)
{

	// 좌표를 배열로 변환
	x = x / 32;
	y = y / 32;

	if (OMok[y][x] != -1){
		return FALSE;	// 이미 돌이 있다면 false
	}

	OMok[y][x] = Turn;	// 돌을 둠


	return true;
}

int check()
{
	int iCheck;

	for (int i = 0; i<MAX_BOARD; i++){
		for (int j = 0; j<MAX_BOARD; j++){
			iCheck = 0;
			// 가로의 오목여부 검사
			for (int k = 0; (j + 5 < MAX_BOARD) && (k < 5); k++)
			{		
				if ((Turn) == (OMok[i][j + k]))
					iCheck++;
			}
			if (iCheck == 5)
				return Turn;
			
			iCheck = 0;

			// (동남쪽)대각선의 오목여부 검사
			for (int k = 0; (i + 5 < MAX_BOARD) && (j + 5 < MAX_BOARD) && (k < 5); k++)
			{
				if ((Turn) == (OMok[i + k][j + k]))
					iCheck++;
			}
			if (iCheck == 5)
				return Turn;
			

			iCheck = 0;
			// (동서쪽)대각선의 오목여부 검사
			for (int k = 0; (i + 5 < MAX_BOARD) && (j - 5 >= 0) && (k < 5); k++)
			{
				if ((Turn) == (OMok[i + k][j - k]))
					iCheck++;
			}
			if (iCheck == 5)
				return Turn;
			
			iCheck = 0;

			// 세로의 오목여부 검사
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
	int iCheck;		// 오목인지 여부
	int OMok_check;	// 돌이 놓여 있는지 여부
	RECT rt;

	if (x > 0 && x < 600 && y > 0 && y < 600)
	{
		AdPoint(x, y);
		OMok_check = put_OMok(x, y);

		if (OMok_check == TRUE)
		{	
			// 돌이 없는 경우
			SetRect(&rt, pX, pY, pX + 31, pY + 31);
			InvalidateRect(hWindow, &rt, FALSE);

			// 오목의 여부를 점검
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