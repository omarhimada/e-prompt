// eprompt.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "eprompt.h"
#include <algorithm>
#include <string>
#include <vector>

#include <cctype>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <unordered_set>      


#define MAX_LOADSTRING 100
#define ID_INPUT_EDIT 1001
#define ID_REVERSE_BUTTON 1002
#define ID_OUTPUT_EDIT 1003
#define BUTTON_HEIGHT 40

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hInputEdit;                                // Input text area
HWND hReverseButton;                            // Reverse button
HWND hOutputEdit;                               // Output text area

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                ResizeControls(HWND hWnd);
void                ReverseString();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_EPROMPT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EPROMPT));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
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
ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EPROMPT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_EPROMPT);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	// Create input text area (multiline edit control)
	hInputEdit = CreateWindowExW(
		WS_EX_CLIENTEDGE,
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
		0, 0, 0, 0,
		hWnd,
		(HMENU)ID_INPUT_EDIT,
		hInstance,
		nullptr);

	// Create reverse button
	hReverseButton = CreateWindowW(
		L"BUTTON",
		L"Sort Prompt",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 0, 0, BUTTON_HEIGHT,
		hWnd,
		(HMENU)ID_REVERSE_BUTTON,
		hInstance,
		nullptr);

	// Create output text area (multiline edit control, read-only)
	hOutputEdit = CreateWindowExW(
		WS_EX_CLIENTEDGE,
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		0, 0, 0, 0,
		hWnd,
		(HMENU)ID_OUTPUT_EDIT,
		hInstance,
		nullptr);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: ResizeControls(HWND)
//
//  PURPOSE: Resize controls based on window size
//
void ResizeControls(HWND hWnd) {
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	int clientWidth = rcClient.right - rcClient.left;
	int clientHeight = rcClient.bottom - rcClient.top;

	// Calculate heights - button stays static, input and output split remaining height
	int remainingHeight = clientHeight - BUTTON_HEIGHT;
	int halfHeight = remainingHeight / 2;

	// Position input edit (top half)
	SetWindowPos(hInputEdit, nullptr, 0, 0, clientWidth, halfHeight, SWP_NOZORDER);

	// Position button (middle)
	SetWindowPos(hReverseButton, nullptr, 0, halfHeight, clientWidth, BUTTON_HEIGHT, SWP_NOZORDER);

	// Position output edit (bottom half)
	SetWindowPos(hOutputEdit, nullptr, 0, halfHeight + BUTTON_HEIGHT, clientWidth, halfHeight, SWP_NOZORDER);
}

//
//  FUNCTION: ReverseString()
//
//  PURPOSE: Reverse the string from input and display in output
//
void ReverseString() {
	// Get length of input text
	int length = GetWindowTextLengthW(hInputEdit);

	if (length > 0) {
		// Allocate buffer for input text
		std::wstring inputText(length + 1, L'\0');
		GetWindowTextW(hInputEdit, inputText.data(), length + 1);
		inputText.resize(length); // Remove extra null terminator

		struct Prompt {
			std::wstring original;
			std::wstring text;
			double weight;
		};

		auto trim = [](std::wstring s) {
			const wchar_t* ws = L" \t\r\n";
			size_t first = s.find_first_not_of(ws);
			if (first == std::wstring::npos)
				return std::wstring();
			return s.substr(first, s.find_last_not_of(ws) - first + 1);
		};

		auto lowercase = [](std::wstring s) {
			std::transform(s.begin(), s.end(), s.begin(), towlower);
			return s;
		};

		std::vector<Prompt> prompts;

		std::wstringstream ss(inputText);
		std::wstring token;

		while (std::getline(ss, token, L',')) {
			token = trim(token);
			if (token.empty())
				continue;

			Prompt p{ token, token, 1.0 };

			// (text:1.2)
			if (token.size() >= 5 && token.front() == L'(' && token.back() == L')') {
				size_t colon = token.rfind(L':');
				if (colon != std::wstring::npos) {
					try {
						p.text = trim(token.substr(1, colon - 1));
						p.weight = std::stod(token.substr(colon + 1, token.size() - colon - 2));
						prompts.push_back(std::move(p));
						continue;
					}
					catch (...) {}
				}
			}

			// (((text)))
			size_t open = 0, close = 0;
			while (open < token.size() && token[open] == L'(') ++open;
			while (close < token.size() && token[token.size() - 1 - close] == L')') ++close;

			if (open > 0 && open == close) {
				p.text = trim(token.substr(open, token.size() - open - close));
				p.weight = 1.0 + open * 0.1;
			}

			prompts.push_back(std::move(p));
		}

		// Deduplication
		{
			std::vector<std::wstring> seen_texts;
			std::vector<Prompt> deduped;
			deduped.reserve(prompts.size());

			for (const auto& p : prompts) {
				std::wstring lower = lowercase(p.text);
				bool found = false;
				for (const auto& s : seen_texts) {
					if (lower == s) {
						found = true;
						break;
					}
				}
				if (!found) {
					seen_texts.push_back(lower);
					deduped.push_back(p);
				}
			}
			prompts = std::move(deduped);
		}

		// Sort
		std::sort(prompts.begin(), prompts.end(),
			[&](const Prompt& a, const Prompt& b) {
			if (a.weight != b.weight)
				return a.weight > b.weight;
			return lowercase(a.text) < lowercase(b.text);
		});

		inputText.clear();
		for (size_t i = 0; i < prompts.size(); ++i) {
			if (i)
				inputText += L", ";
			inputText += prompts[i].original;
		}


		SetWindowTextW(hOutputEdit, inputText.c_str());
	}
	else {
		// Clear output if input is empty
		SetWindowTextW(hOutputEdit, L"");
	}
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		// Initial resize when window is created
		ResizeControls(hWnd);
		break;
	case WM_SIZE:
		// Resize controls when window is resized
		ResizeControls(hWnd);
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId) {
		case ID_REVERSE_BUTTON:
			ReverseString();
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
		BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
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

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}