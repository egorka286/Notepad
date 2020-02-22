#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK OpenDlgFunc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SaveDlgFunc(HWND, UINT, WPARAM, LPARAM);

HWND hWnd;
HWND hEdit;

TCHAR CurrentFile[80] = TEXT("Безымянный");
BOOL FileChanged = FALSE;

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdShow, int nCmdShow)
{
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

	if (!RegisterClassExW(&wcl))
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

	switch (uMessage)
	{
	case WM_PAINT:
		TCHAR Caption[80];
		_tcscpy(Caption, CurrentFile);
		_tcscat(Caption, TEXT(" - Блокнот"));
		SetWindowText(hWnd, Caption);
		GetClientRect(hWnd, &rect);
		MoveWindow(hEdit, 0, 0, rect.right, rect.bottom, 0);
		UpdateWindow(hEdit);
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE)
			FileChanged = TRUE;

		switch (LOWORD(wParam))
		{
		case ID_OPEN:
			if (FileChanged)
				MessageBox(hWnd, TEXT("Файл изменён"), 0, 0);
			return DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_OPEN), NULL, OpenDlgFunc);
			break;

		case ID_SAVE:
			if (FileChanged)
				MessageBox(hWnd, TEXT("Файл изменён"), 0, 0);
			return DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SAVE), NULL, SaveDlgFunc);
			break;

		case ID_EXIT:
			if (FileChanged)
				MessageBox(hWnd, TEXT("Файл изменён"), 0, 0);
			PostQuitMessage(0);
			break;

		case ID_ABOUT:
			MessageBox(hWnd, TEXT("Петренко Е.В.\t2020г."), TEXT("О программе"), MB_ICONINFORMATION | MB_OK);
			break;
		}
		break;

	/*case WM_DESTROY:
		if (FileChanged)
			MessageBox(hWnd, TEXT("Файл изменён"), 0, 0);
		PostQuitMessage(0);
		break;*/

	case WM_CLOSE:
		if (FileChanged)
		{
			int s = MessageBox(hWnd, TEXT("Файл не сохранён.\n\nСохранить?"), 0, MB_ICONWARNING | MB_YESNOCANCEL);
			switch (s)
			{
			case IDYES:
			{
				// save
				TCHAR FileName[80];
				char Buff[10240] = { 0 };
				int size;
				DWORD writen = 0;
				HANDLE hFile;

				_tcscpy(FileName, CurrentFile);
				hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					size = GetWindowTextLengthA(hEdit) + 1;
					if (size > 10240)
						size = 10240;
					GetWindowTextA(hEdit, Buff, size);
					WriteFile(hFile, Buff, size, &writen, NULL);
					_tcscpy(CurrentFile, FileName);
					SendMessage(hWnd, WM_PAINT, 0, 0);
					CloseHandle(hFile);
				}
				PostQuitMessage(0);
			}
				break;
			case IDNO:
				PostQuitMessage(0);
				break;
			}
		}
		else
			PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
	return 0;
}

BOOL CALLBACK OpenDlgFunc(HWND hWndd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	static HWND edit;

	switch (uMessage)
	{
	case WM_INITDIALOG:
		edit = GetDlgItem(hWndd, IDC_FILE);
		SetFocus(edit);
		break;

	case WM_COMMAND:

		switch (wParam)
		{
		case IDOK:
		{
			TCHAR FileName[80];
			char Buff[10240] = { 0 };
			DWORD read = 0;
			HANDLE hFile;

			GetWindowText(edit, FileName, 80);
			if (lstrlen(FileName) == 0)
				MessageBox(hWndd, TEXT("Введите имя файла"), TEXT("Ошибка"), MB_ICONEXCLAMATION | MB_OK);
			else
			{
				hFile = CreateFile(FileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					ReadFile(hFile, Buff, sizeof(Buff), &read, NULL);
					SetWindowTextA(hEdit, Buff);
					_tcscpy(CurrentFile, FileName);
					SendMessage(hWnd, WM_PAINT, 0, 0);
					CloseHandle(hFile);
					FileChanged = FALSE;
					EndDialog(hWndd, 0);
				}
				else
					MessageBox(hWndd, TEXT("Файл с указанным именем не найден"), TEXT("Ошибка"), MB_ICONEXCLAMATION | MB_OK);
			}
		}
		break;

		case IDCANCEL:
			EndDialog(hWndd, 0);
			break;
		}
		return TRUE;
		break;

	case WM_CLOSE:
		EndDialog(hWndd, 0);
		return TRUE;
		break;
	}
	return FALSE;
}

BOOL CALLBACK SaveDlgFunc(HWND hWndd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	static HWND edit;

	switch (uMessage)
	{
	case WM_INITDIALOG:
		edit = GetDlgItem(hWndd, IDC_FILE);
		SetWindowText(edit, CurrentFile);
		SetFocus(edit);
		break;

	case WM_COMMAND:

		switch (wParam)
		{
		case IDOK:
		{
			TCHAR FileName[80];
			char Buff[10240] = { 0 };
			int size;
			DWORD writen = 0;
			HANDLE hFile;

			GetWindowText(edit, FileName, 80);
			if (lstrlen(FileName) == 0)
				MessageBox(hWndd, TEXT("Введите имя файла"), TEXT("Ошибка"), MB_ICONEXCLAMATION | MB_OK);
			else
			{
				hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					size = GetWindowTextLengthA(hEdit) + 1;
					if (size > 10240)
						size = 10240;
					GetWindowTextA(hEdit, Buff, size);
					WriteFile(hFile, Buff, size, &writen, NULL);
					_tcscpy(CurrentFile, FileName);
					SendMessage(hWnd, WM_PAINT, 0, 0);
					CloseHandle(hFile);
					FileChanged = FALSE;
					EndDialog(hWndd, 0);
				}
				else
					MessageBox(hWndd, TEXT("Ошибка при сохранении файла"), TEXT("Ошибка"), MB_ICONEXCLAMATION | MB_OK);
			}
		}
		break;

		case IDCANCEL:
			EndDialog(hWndd, 0);
			break;
		}
		return TRUE;
		break;

	case WM_CLOSE:
		EndDialog(hWndd, 0);
		return TRUE;
		break;
	}
	return FALSE;
}
