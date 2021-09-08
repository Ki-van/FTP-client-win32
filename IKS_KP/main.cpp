// IKS_KP.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "IKS_KP.h"


#define MAX_LOADSTRING      100
#define IDC_LISTVIEW_FILES	2001

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWndMain;
FTPClient command, data;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ServerChoose(HWND, UINT, WPARAM, LPARAM);
BOOL                OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);

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
		
		case IDM_NEWCONNECT: {
			INT_PTR result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SERVER_CHOOSE), hWnd, ServerChoose, NULL);
			
			if (result == TRUE) {
				int a1, a2, a3, a4, p1, p2;
				char buf[1024];
				command.SendMsg("PASV\r\n", 6);
				command.RecvMsg(buf, 1024);
				sscanf_s(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", &a1, &a2, &a3, &a4, &p1, &p2);
				int dataPort = (p1 * 256) + p2;

				//Opening new data connection to appempt to STOR file in server root dir.
				data.Connect(dataPort, command.saddr.sin_addr.s_addr);
				command.SendMsg("LIST\r\n", 6);
				command.RecvMsg();

				data.RecvMsg();
				command.RecvMsg();
			}

		}break;

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
			
			command.Connect(21, ntohl((ULONG)address));
			command.RecvMsg();
			command.RecvMsg();

			char* user = NULL, * password = NULL;

			if (SendDlgItemMessage(hDlg, IDC_CHECK_ANON, BM_GETCHECK, 0, 0))
			{
				user = new char[] {"anonymous"};
				password = new char[] {"1@1"};
			}
			
			char userCommand[256];
			sprintf_s(userCommand, "%s %s\r\n", "USER", user);
			command.SendMsg(userCommand, strlen(userCommand));
			command.RecvMsg();
			

			char passwordCommand[256];
			sprintf_s(passwordCommand, "%s %s\r\n", "PASS", password);
			command.SendMsg(passwordCommand, strlen(passwordCommand));
			command.RecvMsg();

			TCHAR text[] = TEXT("Соединение успешно установлено");
			TCHAR caption[] = TEXT("Успех");
			MessageBox(hDlg, text, NULL, MB_OK);
			delete[] user, password;

			EndDialog(hDlg, 1);
			return (INT_PTR)TRUE;
			
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
