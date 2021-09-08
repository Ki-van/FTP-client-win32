// IKS_KP.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "IKS_KP.h"


#define MAX_LOADSTRING      100
#define IDC_LISTVIEW_FILES	2001

#define DATA_BUFFER_SIZE 1024000
#define COMMAND_BUFFER_SIZE 10240

// Structs

struct ThreadStruct {
	HANDLE hRecieveEvent;
	HANDLE hWaitEvent;
	LPVOID pvRecieveBuffer;
	DWORD dwBytesRecieve;
	SOCKET* socket;
} ReciveCommandThreadStruct, ReciveDataThreadStruct;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
LPVOID pvReciveDataBuffer = NULL;
LPVOID pvReciveCommandBuffer = NULL;
HANDLE hDataReciveEvent, hWaitDataEvent, hWaitConnectEvent, hCommandReciveEvent, hWaitCommandEvent;
SOCKET sCommand;
SOCKET sData;
HWND hWndMain;
WSADATA wlib;
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ServerChoose(HWND, UINT, WPARAM, LPARAM);
BOOL                OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
DWORD WINAPI        RecieveThread(PVOID);
DWORD				WaitAnswerRecieve(DWORD);
VOID				SendCommandInSock(SOCKET* pSock, char* pOutBuffer, int size);
VOID				ClearThreadStrucBuffer(ThreadStruct* ts, int bufferSize);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_IKSKP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IKSKP));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IKSKP));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_IKSKP);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	hWndMain = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWndMain)
	{
		return FALSE;
	}

	ShowWindow(hWndMain, nCmdShow);
	UpdateWindow(hWndMain);

	pvReciveDataBuffer = VirtualAlloc(NULL, DATA_BUFFER_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	pvReciveCommandBuffer = VirtualAlloc(NULL, COMMAND_BUFFER_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (pvReciveCommandBuffer == NULL || pvReciveDataBuffer == NULL)
	{
		return FALSE;
	}

	hDataReciveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hWaitDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hCommandReciveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hWaitConnectEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hWaitCommandEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	if (hDataReciveEvent == NULL)
	{
		return FALSE;
	}

	
	ReciveCommandThreadStruct = {
		hCommandReciveEvent,
		hWaitCommandEvent,
		pvReciveCommandBuffer,
		0,
		&sCommand
	};

	ReciveDataThreadStruct = {
		hDataReciveEvent,
		hWaitDataEvent,
		pvReciveDataBuffer,
		0,
		&sData
	};

	CreateThread(NULL, NULL, RecieveThread, (PVOID)&ReciveCommandThreadStruct, NORMAL_PRIORITY_CLASS, NULL);
	CreateThread(NULL, NULL, RecieveThread, (PVOID)&ReciveDataThreadStruct, NORMAL_PRIORITY_CLASS, NULL);

	if (WSAStartup(MAKEWORD(2, 2), &wlib) != 0)
	{
		return FALSE;
	}

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		
		case IDM_NEWCONNECT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_SERVER_CHOOSE), hWnd, ServerChoose);
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
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	HWND hListView = CreateWindowEx(0, WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
		10, 40, 500, 600, hwnd, (HMENU)IDC_LISTVIEW_FILES, lpCreateStruct->hInstance, NULL);

	return TRUE;
}

// Message handler for about box.
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

// Message handler for new connection box
INT_PTR CALLBACK ServerChoose(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	
	case WM_INITDIALOG:
	{
		HWND hWndComboBox = GetDlgItem(hDlg, IDC_ADDRESS_TYPE);

		TCHAR addressTypes[2][13] =
		{
			TEXT("IPv4"), TEXT("Доменное имя")
		};

		TCHAR A[16];
		int  k = 0;

		memset(&A, 0, sizeof(A));

		for (k = 0; k < 2; ++k)
		{
			wcscpy_s(A, sizeof(A) / sizeof(TCHAR), (TCHAR*)addressTypes[k]);

			// Add string to combobox.
			SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
		}

		// Send the CB_SETCURSEL message to display an initial item 
		//  in the selection field  
		SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		HWND hWndIpControl = GetDlgItem(hDlg, IDC_IPADDRESS);
		SendMessage(hWndIpControl, IPM_SETADDRESS, (WPARAM)0, (LPARAM)MAKEIPADDRESS(127, 0, 0, 1));

		HWND hWndCheckBox = GetDlgItem(hDlg, IDC_CHECK_ANON);
		Button_SetCheck(hWndCheckBox, BST_CHECKED);

		SendMessage(hDlg, WM_COMMAND, NULL, MAKELPARAM(0, IDOK));
		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) 
		{
			// If IPv4 selected and int's valid
			HWND hWndIpControl = GetDlgItem(hDlg, IDC_IPADDRESS);
			DWORD address;
			SendMessage(hWndIpControl, IPM_GETADDRESS, (WPARAM)0, (LPARAM)&address);
			
			sockaddr_in saddr;
			saddr.sin_family = AF_INET;
			saddr.sin_port = htons(21);
			saddr.sin_addr.s_addr = ntohl((ULONG)address);

			sCommand = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			
			if (connect(sCommand, (SOCKADDR*)(&saddr), sizeof(saddr)) != 0)
			{
				TCHAR t[] = TEXT("Faild to connect 0((");
				MessageBox(hDlg, t, NULL, MB_OK);
			}
			else 
			{
				WSAEventSelect(sCommand, hCommandReciveEvent, FD_READ);
				WaitAnswerRecieve(5000);
				
				logA("%s", (const char*)pvReciveCommandBuffer);
				ClearThreadStrucBuffer(&ReciveCommandThreadStruct, COMMAND_BUFFER_SIZE);

				HWND hWndCheckBox = GetDlgItem(hDlg, IDC_CHECK_ANON);
				char* user = NULL, * password = NULL;

				if (SendDlgItemMessage(hDlg, IDC_CHECK_ANON, BM_GETCHECK, 0, 0))
				{
					user = new char[] {"anonymous"};
					password = new char[] {"1@1"};
				}


				char userCommand[256];
				sprintf_s(userCommand, "%s %s\r\n", "USER", user);
				SendCommandInSock(&sCommand, userCommand, 256);
				logA("%s", (const char*)pvReciveCommandBuffer);
				ClearThreadStrucBuffer(&ReciveCommandThreadStruct, COMMAND_BUFFER_SIZE);
				//DWORD dwWaitResult = WaitAnswerRecieve(5000);
				/*if (dwWaitResult != 0)
				{
					TCHAR t[] = TEXT("Ошибка регистрации на сервере");
					MessageBox(hDlg, t, NULL, MB_OK);
					return FALSE;
				}*/
				
				char passwordCommand[256];
				sprintf_s(passwordCommand, "%s %s\r\n", "PASSWORD", password);
				SendCommandInSock(&sCommand, passwordCommand, strlen(passwordCommand));
				logA("%s", (const char*)pvReciveCommandBuffer);
				ClearThreadStrucBuffer(&ReciveCommandThreadStruct, COMMAND_BUFFER_SIZE);
				/*WaitAnswerRecieve(5000);
				if (dwWaitResult != 0)
				{
					TCHAR t[] = TEXT("Ошибка регистрации на сервере");
					MessageBox(hDlg, t, NULL, MB_OK);
					return FALSE;
				}*/
				TCHAR text[] = TEXT("Соединение успешно установлено");
				TCHAR caption[] = TEXT("Успех");
				MessageBox(hDlg, text, NULL, MB_OK);
				delete[] user, password;

				EndDialog(hDlg, 0);
				return (INT_PTR)TRUE;
			}
			
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

VOID ClearThreadStrucBuffer(ThreadStruct* ts, int bufferSize)
{
	memset(ts->pvRecieveBuffer, 0, bufferSize);
	ts->dwBytesRecieve = 0;
}

DWORD WaitAnswerRecieve(DWORD timeToWait) 
{
	DWORD dwWaitResult = WaitForSingleObject(hWaitCommandEvent, timeToWait);
	if (dwWaitResult != WAIT_OBJECT_0) {
		TCHAR t[] = TEXT("Ответ от сервера не получен ((");
		MessageBox(hWndMain, t, NULL, MB_OK);
		
	} 
	ResetEvent(hWaitCommandEvent);
	return dwWaitResult;
}

DWORD WINAPI RecieveThread(PVOID pvParam)
{
	ThreadStruct* pThreadStruct = (ThreadStruct*)pvParam;

	while (true) {
		WaitForSingleObject(pThreadStruct->hRecieveEvent, INFINITE);
		pThreadStruct->dwBytesRecieve += recv(
			*pThreadStruct->socket,
			(char*)pThreadStruct->pvRecieveBuffer + pThreadStruct->dwBytesRecieve,
			COMMAND_BUFFER_SIZE,
			0);

		if (pThreadStruct == &ReciveCommandThreadStruct) {
				if (*((char*)pThreadStruct->pvRecieveBuffer + pThreadStruct->dwBytesRecieve - 1) == '\n') {
					SetEvent(pThreadStruct->hWaitEvent);
					ResetEvent(pThreadStruct->hRecieveEvent);
				}
		} 
	}
	return (DWORD)TRUE;
}

VOID SendCommandInSock(SOCKET* pSock, char  * pOutBuffer, int size) 
{
	send(*pSock, pOutBuffer, size, 0);
	DWORD dwWaitResult;
	while (true)
	{
		dwWaitResult = WaitAnswerRecieve(5000);

		if (dwWaitResult != 0)
			return;

		char* pAnswerBegin = (char*)pvReciveCommandBuffer + ReciveCommandThreadStruct.dwBytesRecieve - 2;
		for (pAnswerBegin; pAnswerBegin > pvReciveCommandBuffer && *pAnswerBegin != '\n'; pAnswerBegin--);

		pAnswerBegin++;

		if (*pAnswerBegin == '1')
		{
			continue;
		}
		if (*pAnswerBegin > '3')
		{
			TCHAR t[] = TEXT("Команда сервером не выполнена (((");
			MessageBox(hWndMain, t, NULL, MB_OK);
			break;
		}

		break;
	}
}

VOID CreateListenSock() 
{
	sData = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	WSAEventSelect(sData, hDataReciveEvent, FD_ACCEPT | FD_READ | FD_CLOSE);

	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = 0;
	saddr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(sData, (SOCKADDR *)&saddr, sizeof(saddr));
	int namelen = sizeof(saddr);
	int sockname = getsockname(sData, (SOCKADDR*)&saddr, &namelen);
	listen(sData, 1);
}