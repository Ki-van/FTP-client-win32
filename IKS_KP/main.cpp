// IKS_KP.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "IKS_KP.h"


#define MAX_LOADSTRING      100
#define IDC_LISTVIEW_FILES	2001
#define IDC_SAVEFILEBTN		2002

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
VOID				PopulateFileList();
int					ExtractFileNameFromMLST(char const* filefacts, char* filename, int filenamesize);
int					ExtractFactFromMLST(char const* filefacts, const char* fact, char* result);
VOID				OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
VOID				EnterPassivMode();
VOID				OnNotify(HWND hwnd, LPNMHDR lpnmhdr);

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
		HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
	case WM_NOTIFY: {
		OnNotify(hWnd, (LPNMHDR)lParam);
		return 0;
	}
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
	CreateWindowEx(0, TEXT("Static"), TEXT("Список файлов"), WS_CHILD | WS_VISIBLE | SS_CENTER,
		10, 10, 500, 20, hwnd, NULL, lpCreateStruct->hInstance, NULL);
	HWND hListView = CreateWindowEx(0, WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
		10, 40, 500, 600, hwnd, (HMENU)IDC_LISTVIEW_FILES, lpCreateStruct->hInstance, NULL);
	ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	WCHAR szText[256] = { L'\0' };
	LVCOLUMN lvCol;

	//Name column
	lvCol.mask = LVCF_WIDTH | LVCF_TEXT;
	lvCol.cx = 380;
	lvCol.pszText = szText;
	swprintf_s(szText, L"Имя\0");
	ListView_InsertColumn(hListView, 0, &lvCol);

	//PID column
	lvCol.cx = 100;
	swprintf_s(szText, L"Время последнего изменения\0");
	ListView_InsertColumn(hListView, 1, &lvCol);


	CreateWindowEx(0, WC_BUTTON, TEXT(" Скачать файл"), WS_CHILD | WS_VISIBLE | WS_DISABLED | BS_TEXT,
		365, 650, 145, 40, hwnd, (HMENU)IDC_SAVEFILEBTN, lpCreateStruct->hInstance, NULL);
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
			MessageBox(hDlg, text, caption, MB_OK);
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
/// <summary>
/// Require mlst format
/// </summary>
/// <param name="hListView"></param>
/// <param name="mlst"></param>
VOID PopulateFileList() {
	
	HWND hListView = GetDlgItem(hWndMain, IDC_LISTVIEW_FILES);
	SendMessage(hListView, WM_SETREDRAW, FALSE, NULL);
	
	char filefacts[1024];

	while(data.RecvNextMLST(filefacts, 1024)) {
		char szfilenameA[512];
		int filenamelen = ExtractFileNameFromMLST(filefacts, szfilenameA, 512);
		::std::wstring szfilenameW;
		szfilenameW.resize(1024);

		int r = MultiByteToWideChar(CP_UTF8, 0, szfilenameA, filenamelen, &szfilenameW[0], (int)szfilenameW.size());

 		LVITEM lvItem = { LVIF_TEXT | LVIF_IMAGE };
		lvItem.iItem = ListView_GetItemCount(hListView);
		lvItem.pszText = &szfilenameW[0];
		lvItem.iItem = ListView_InsertItem(hListView, &lvItem);
		if (lvItem.iItem != -1) {
			char szFileModify[128];
			ExtractFactFromMLST(filefacts, "modify", szFileModify);

			//swprintf_s(sz, L"%d", pe.th32ProcessID);
			//ListView_SetItemText(hListView, lvItem.iItem, 1, sz);
		}
	}
	SendMessage(hListView, WM_SETREDRAW, TRUE, NULL);
	RedrawWindow(hListView, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}

int ExtractFileNameFromMLST(char const* filefacts, char *filename, int filenamesize)
{
	int i = strnlen_s(filefacts, filenamesize) - 1;
	while (filefacts[i] != ';')
		i--;
	i++;

	int filenameLen = 0;
	while (filefacts[i] != '\r')
	{
		filename[filenameLen] = filefacts[i];
		filenameLen++;
		i++;
	}

	filename[filenameLen] = '\0';
	filenameLen++;

	return filenameLen;
}

int	ExtractFactFromMLST(char const* szFilefacts, const char* szFact, char* szFactVal)
{
	const char *pos = strstr(szFilefacts, szFact);
	if (pos == NULL)
		return 0;
	pos += strlen(szFact) + 1;
	int i = 0;
	
	for (; *pos != ';'; szFactVal[i] = *pos, i++, pos++);
	return i;
}

VOID OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDC_SAVEFILEBTN: {
		if (codeNotify == BN_CLICKED) {
			
			::std::wstring szFileNameW;
			
			szFileNameW.resize(128);
			HWND hLsitView = GetDlgItem(hWndMain, IDC_LISTVIEW_FILES);
			LVITEM lvItem = { LVFIF_TEXT };
			lvItem.iItem = ListView_GetNextItem(hLsitView, -1, LVNI_SELECTED);
			lvItem.iSubItem = 0;
			lvItem.pszText = &szFileNameW[0];
			lvItem.cchTextMax = 128;
			ListView_GetItem(hLsitView, &lvItem);
			EnterPassivMode();

			char szFileNameA[128];
			WideCharToMultiByte(CP_UTF8, 0, &szFileNameW[0], szFileNameW.size(), szFileNameA, 128, NULL, NULL);
			char szRetrCommand[256];
			
			sprintf_s(szRetrCommand, "%s %s\r\n", "RETR", szFileNameA);
			command.SendMsg(szRetrCommand, strlen(szRetrCommand));
			
			command.RecvMsg();
			data.SaveFile(&szFileNameW[0]);
			TCHAR text[256];
			wsprintfW(text, L"Файл %s успешно скачан", &szFileNameW[0]);
			TCHAR caption[] = TEXT("Успех");
			MessageBox(hWndMain, text, caption, MB_OK);
			data.CloseCon();
			command.RecvMsg();
		}
	} break;
	case IDM_NEWCONNECT: {
		INT_PTR result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SERVER_CHOOSE), hwnd, ServerChoose, NULL);

		if (result == TRUE) {
			EnterPassivMode();
			command.SendMsg("MLSD\r\n", 6);
			command.RecvMsg();

			PopulateFileList();
			data.CloseCon();
			command.RecvMsg();
		}

	}break;
	case IDM_ABOUT:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
		break;
	case IDM_EXIT:
		DestroyWindow(hwnd);
		break;
	}
}

VOID EnterPassivMode() {
	int a1, a2, a3, a4, p1, p2;
	char buf[2048];
	command.SendMsg("PASV\r\n", 6);
	command.RecvMsg(buf, 2048);
	sscanf_s(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", &a1, &a2, &a3, &a4, &p1, &p2);
	int dataPort = (p1 * 256) + p2;

	//Opening new data connection to appempt to STOR file in server root dir.
	data.Connect(dataPort, command.saddr.sin_addr.s_addr);
}

VOID OnNotify(HWND hwnd, LPNMHDR lpnmhdr)
{
	switch (lpnmhdr->code)
	{
	case NM_CLICK: {
		if (lpnmhdr->idFrom == IDC_LISTVIEW_FILES)
		{
			EnableWindow(GetDlgItem(hWndMain, IDC_SAVEFILEBTN), TRUE);
		}
	} break;
	}
}