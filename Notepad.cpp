#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK OpenDlgFunc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SaveDlgFunc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK CreateDlgFunc(HWND, UINT, WPARAM, LPARAM);

int PromptSave(HWND);
void SaveFile(HWND hWnd, const TCHAR* FileName);
void LoadFile(HWND hWnd, const TCHAR* FileName);

HWND hEdit;
TCHAR CurrentFile[80] = TEXT("Безымянный.txt");
BOOL FileChanged = FALSE;

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdShow, int nCmdShow)
{
	HWND hWnd;
	TCHAR szClassName[] = TEXT("Notepad");
	MSG lpMsg;
	WNDCLASSEX wcl;
	RECT rect;
	int H, W, h, w, x, y;

	wcl.cbSize = sizeof(wcl);
	wcl.style = CS_HREDRAW | CS_VREDRAW;
	wcl.lpfnWndProc = WindowProc;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = hInst;
	wcl.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NOTEPAT_ICON));
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcl.lpszMenuName = TEXT("MAIN_MENU");
	wcl.lpszClassName = szClassName;
	wcl.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NOTEPAD_SMICON));

	if (!RegisterClassEx(&wcl))
		return 0;

	H = GetSystemMetrics(SM_CXSCREEN);
	W = GetSystemMetrics(SM_CYSCREEN);
	h = H / 2;
	w = W / 2;
	x = h / 2;
	y = w / 2;

	hWnd = CreateWindowEx(0, szClassName, TEXT("Блокнот"), WS_OVERLAPPEDWINDOW, x, y, h, w, NULL, NULL, hInst, NULL);
	GetClientRect(hWnd, &rect);
	hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 0, 0, rect.right, rect.bottom, hWnd, NULL, hInst, NULL);
	SetFocus(hEdit);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&lpMsg, NULL, 0, 0))
	{
		TranslateMessage(&lpMsg);
		DispatchMessageW(&lpMsg);
	}
	return lpMsg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	TCHAR Caption[80];

	switch (uMessage)
	{
	case WM_PAINT:
		lstrcpy(Caption, CurrentFile);
		lstrcat(Caption, TEXT(" - Блокнот"));
		SetWindowText(hWnd, Caption);
		GetClientRect(hWnd, &rect);
		MoveWindow(hEdit, 0, 0, rect.right, rect.bottom, 0);
		UpdateWindow(hEdit);
		break;

	case WM_COMMAND:
		if (lParam == (LPARAM)hEdit && HIWORD(wParam) == EN_CHANGE)
			FileChanged = TRUE;

		switch (LOWORD(wParam))
		{
		case ID_CREATE:
			return DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CREATE), hWnd, CreateDlgFunc);

		case ID_OPEN:
			return DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPEN), hWnd, OpenDlgFunc);

		case ID_SAVE:
			return DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SAVE), hWnd, SaveDlgFunc);

		case ID_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0);

		case ID_UNDO:
			SendMessage(hEdit, WM_UNDO, 0, 0);
			break;

		case ID_CUT:
			SendMessage(hEdit, WM_CUT, 0, 0);
			break;

		case ID_COPY:
			SendMessage(hEdit, WM_COPY, 0, 0);
			break;

		case ID_PASTE:
			SendMessage(hEdit, WM_PASTE, 0, 0);
			break;

		case ID_DELETE:
			SendMessage(hEdit, WM_CLEAR, 0, 0);
			break;

		case ID_ABOUT:
			MessageBox(hWnd, TEXT("Петренко Е.В.\t2020г."), TEXT("О программе"), MB_ICONINFORMATION | MB_OK);
			break;
		}
		break;

	case WM_CLOSE:
		if (!FileChanged)
			DestroyWindow(hWnd);
		if (IDCANCEL != PromptSave(hWnd))
			DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
	return 0;
}

BOOL CALLBACK OpenDlgFunc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	TCHAR FileName[80];

	switch (uMessage)
	{
	case WM_INITDIALOG:
		SetFocus(GetDlgItem(hDlg, IDC_FILE));
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			if (FileChanged)
				if (IDCANCEL == PromptSave(hDlg))
					SendMessage(hDlg, WM_CLOSE, 0, 0);

			GetDlgItemText(hDlg, IDC_FILE, FileName, sizeof(FileName));
			if (!lstrlen(FileName))
			{
				MessageBox(hDlg, TEXT("Введите имя файла"), 0, MB_ICONERROR | MB_OK);
				SetFocus(GetDlgItem(hDlg, IDC_FILE));
			}
			else
			{
				LoadFile(hDlg, FileName);
				EndDialog(hDlg, 0);
				return TRUE;
			}
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}

		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK SaveDlgFunc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	TCHAR FileName[80];

	switch (uMessage)
	{
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hDlg, IDC_FILE), CurrentFile);
		SetFocus(GetDlgItem(hDlg, IDC_FILE));
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_FILE, FileName, sizeof(FileName));
			if (!lstrlen(FileName))
			{
				MessageBox(hDlg, TEXT("Введите имя файла"), 0, MB_ICONERROR | MB_OK);
				SetFocus(GetDlgItem(hDlg, IDC_FILE));
			}
			else
			{
				SaveFile(hDlg, FileName);
				EndDialog(hDlg, 0);
				return TRUE;
			}
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}

		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK CreateDlgFunc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	TCHAR FileName[80];

	switch (uMessage)
	{
	case WM_INITDIALOG:
		SetFocus(GetDlgItem(hDlg, IDC_FILE));
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			if (FileChanged)
				if (IDCANCEL == PromptSave(hDlg))
					SendMessage(hDlg, WM_CLOSE, 0, 0);

			GetDlgItemText(hDlg, IDC_FILE, FileName, sizeof(FileName));
			if (!lstrlen(FileName))
			{
				MessageBox(hDlg, TEXT("Введите имя файла"), 0, MB_ICONERROR | MB_OK);
				SetFocus(GetDlgItem(hDlg, IDC_FILE));
			}
			else
			{
				SetWindowText(hEdit, TEXT(""));
				SaveFile(hDlg, FileName);
				EndDialog(hDlg, 0);
				return TRUE;
			}
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}

		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}


int PromptSave(HWND hWnd)
{
	TCHAR msg[80];
	int R;

	wsprintf(msg, TEXT("Файл %s был изменён. Сохранить?"), CurrentFile);
	R = MessageBox(hWnd, msg, TEXT("Внимание"), MB_ICONEXCLAMATION | MB_YESNOCANCEL);
	if (R == IDYES)
		SaveFile(hWnd, CurrentFile);
	return R;
}

void SaveFile(HWND hWnd, const TCHAR* FileName)
{
	HANDLE hFile;
	char Buff[10240];
	DWORD writen;
	int size;

	hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		size = GetWindowTextLengthA(hEdit) + 1;
		if (size > 10240)
			size = 10240;
		GetWindowTextA(hEdit, Buff, size);
		WriteFile(hFile, Buff, size, &writen, NULL);
		CloseHandle(hFile);
		lstrcpy(CurrentFile, FileName);
		SendMessage(GetParent(hWnd), WM_PAINT, (WPARAM)0, (LPARAM)0);
		FileChanged = FALSE;
	}
	else
		MessageBox(hWnd, TEXT("Ошибка при сохранении файла"), 0, MB_ICONEXCLAMATION | MB_OK);
}

void LoadFile(HWND hWnd, const TCHAR* FileName)
{
	HANDLE hFile;
	char Buff[10240];
	DWORD read;

	hFile = CreateFile(FileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		ReadFile(hFile, Buff, sizeof(Buff), &read, NULL);
		CloseHandle(hFile);
		SetWindowTextA(hEdit, Buff);
		lstrcpy(CurrentFile, FileName);
		SendMessage(GetParent(hWnd), WM_PAINT, (WPARAM)0, (LPARAM)0);
		FileChanged = FALSE;
	}
	else
		MessageBox(hWnd, TEXT("Файл с указанным именем не найден"), 0, MB_ICONEXCLAMATION | MB_OK);
}
