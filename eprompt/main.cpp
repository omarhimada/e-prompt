// eprompt.cpp : Defines the entry point for the application.
// Sort Stable Diffusion prompts by weight & lexicographically, and remove duplicates

#include "framework.h"
#include <algorithm>
#include <cmath>
#include <cwctype>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <richedit.h>

#include "flatbutton.h"
#include "main.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_EPROMPT, szWindowClass, MAX_LOADSTRING);
	RegisterClass(hInstance);

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
//  FUNCTION: RegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM RegisterClass(HINSTANCE hInstance) {
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

void RegisterWarningWindow(HINSTANCE hInstance) {
	WNDCLASSW wc = {};

	wc.lpfnWndProc = MalformedWindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"WarningWindow";
	wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

	RegisterClassW(&wc);
}

LRESULT CALLBACK InputEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_KEYDOWN) {
		if (wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return 0;
		}
	}

	return CallWindowProc(g_DefaultEditProc, hwnd, msg, wParam, lParam);
}

//
//  FUNCTION: SetRichEditFont(HWND hRichEdit, const wchar_t* faceName, int size, bool bold, bool italic) {
//
//  PURPOSE: Set font for all text in a RichEdit control
//
void SetRichEditFont(HWND hRichEdit, const wchar_t* faceName, int size, bool bold, bool italic) {
	CHARFORMAT2 cf;
	ZeroMemory(&cf, sizeof(CHARFORMAT2));

	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_ITALIC;

	// Set font face name (max 32 chars)
	wcsncpy_s(cf.szFaceName, CF_MAXFACELEN + 1, faceName, CF_MAXFACELEN);

	// Size is in half-points (e.g., 12pt = 240)
	cf.yHeight = size * 20;

	if (bold)   cf.dwEffects |= CFE_BOLD;
	if (italic) cf.dwEffects |= CFE_ITALIC;

	// SCF_ALL applies to all text, SCF_SELECTION only selected text
	SendMessage(hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
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
	
	LoadLibraryW(L"Msftedit.dll");

	RegisterFlatButton(hInstance);
	RegisterWarningWindow(hInstance);

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	g_hTextAreaBgBrush = CreateSolidBrush(RGB(192, 192, 192));

	// Create input text area (multiline edit control)
	hInputEdit = CreateWindowExW(
		0,
		MSFTEDIT_CLASS,
		L"",
		WS_CHILD |
		WS_VISIBLE |
		WS_VSCROLL |
		ES_MULTILINE |
		ES_AUTOVSCROLL |
		ES_WANTRETURN |
		ES_NOHIDESEL,
		0, 0, 0, 0,
		hWnd,
		(HMENU)ID_INPUT_EDIT,
		hInstance,
		nullptr);

	// Create sort prompts button
	hSortPromptButton = CreateFlatButton(
		hWnd,
		ID_SORT_PROMPT_BUTTON,
		L"Sort Prompts",
		0,
		0,
		0,
		BUTTON_HEIGHT);

	FlatButton_SetBackground(hSortPromptButton, RGB(8, 8, 8));
	FlatButton_SetHoverBackground(hSortPromptButton, RGB(64, 64, 64));
	FlatButton_SetPressedBackground(hSortPromptButton, RGB(81, 81, 81));
	FlatButton_SetTextColor(hSortPromptButton,RGB(234, 234, 234));

	// Create copy output button
	hCopyOutputButton = CreateFlatButton(
		hWnd,
		ID_COPY_OUTPUT_BUTTON,
		L"Copy Output",
		0,
		0,
		0,
		BUTTON_HEIGHT); 
	
	FlatButton_SetBackground(hCopyOutputButton, RGB(8, 8, 8));
	FlatButton_SetHoverBackground(hCopyOutputButton, RGB(64, 64, 64));
	FlatButton_SetPressedBackground(hCopyOutputButton, RGB(81, 81, 81));
	FlatButton_SetTextColor(hCopyOutputButton, RGB(234, 234, 234));

	// Create output text area (multiline edit control, read-only)
	hOutputDisplay = CreateWindowExW(
		0,
		MSFTEDIT_CLASS,
		L"",
		WS_CHILD |
		WS_VISIBLE |
		WS_VSCROLL |
		ES_MULTILINE |
		ES_AUTOVSCROLL |
		ES_READONLY |
		ES_NOHIDESEL,
		0, 0, 0, 0,
		hWnd,
		(HMENU)ID_OUTPUT_EDIT,
		hInstance,
		nullptr);

	g_DefaultEditProc = (WNDPROC)SetWindowLongPtr(
		hInputEdit,
		GWLP_WNDPROC,
		(LONG_PTR)InputEditProc);

	// TODO (event mask, notifications)
	/*SendMessage(
		hInputEdit,
		EM_SETEVENTMASK,
		0,
		ENM_CHANGE | ENM_SELCHANGE);*/

	/*ENM_SCROLL
	ENM_KEYEVENTS
	ENM_MOUSEEVENTS*/


	// Loads an example set of negative prompts for sorting
	LoadExampleNegativePrompt();

	// Disable RichEdit's URL detection
	SendMessage(
		hInputEdit,
		EM_AUTOURLDETECT,
		FALSE,
		0);


	// Set the font for both RichEdit controls 
	SetRichEditFont(hInputEdit, L"Consolas", 12, FALSE, FALSE);
	SetRichEditFont(hOutputDisplay, L"Consolas", 12, FALSE, FALSE);

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

	// Each button takes up half the width of the window
	int buttonWidth = clientWidth / 2;

	// Position input edit (top half)
	SetWindowPos(hInputEdit, nullptr, 0, 0, clientWidth, halfHeight, SWP_NOZORDER);

	// Position button (middle)
	SetWindowPos(hSortPromptButton, nullptr, 0, halfHeight, buttonWidth, BUTTON_HEIGHT, SWP_NOZORDER);

	// Position second button (middle)
	SetWindowPos(hCopyOutputButton, nullptr, buttonWidth, halfHeight, clientWidth - buttonWidth, BUTTON_HEIGHT, SWP_NOZORDER);

	// Position output edit (bottom half)
	SetWindowPos(hOutputDisplay, nullptr, 0, halfHeight + BUTTON_HEIGHT, clientWidth, halfHeight, SWP_NOZORDER);
}

LRESULT CALLBACK MalformedWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {

	case WM_COMMAND:
		if (LOWORD(wParam) == ID_MALFORMED_CLOSE_BUTTON) {
			DestroyWindow(hWnd);
		}
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		RECT rc;
		GetClientRect(hWnd, &rc);

		HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hdc, &rc, brush);
		DeleteObject(brush);

		SetTextColor(hdc, RGB(234, 234, 234));
		SetBkMode(hdc, TRANSPARENT);

		RECT textRect = rc;
		textRect.left += 20;
		textRect.right -= 20;
		textRect.top += 20;
		textRect.bottom -= 0;

		DrawTextW(
			hdc,
			malformedMessage.c_str(),
			-1,
			&textRect,
			DT_LEFT | DT_WORDBREAK
		);

		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:
		hMalformedWindow = nullptr;
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void ShowWarningWindow(const std::wstring& message) {
	malformedMessage = message;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int windowWidth = 580;
	int windowHeight = 400;

	int x = (screenWidth - windowWidth) / 2;
	int y = (screenHeight - windowHeight) / 2;

	hMalformedWindow = CreateWindowExW(
		0,
		L"WarningWindow",
		L"Prompt Warning",
		WS_POPUP | WS_CAPTION,
		x,
		y,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		hInst,
		nullptr
	);

	hMalformedCloseButton = CreateFlatButton(
		hMalformedWindow,
		ID_MALFORMED_CLOSE_BUTTON,
		L"Close",
		0,
		320,
		580,
		40
	);

	FlatButton_SetBackground(hMalformedCloseButton, RGB(8, 8, 8));
	FlatButton_SetHoverBackground(hMalformedCloseButton, RGB(64, 64, 64));
	FlatButton_SetPressedBackground(hMalformedCloseButton, RGB(81, 81, 81));
	FlatButton_SetTextColor(hMalformedCloseButton, RGB(234, 234, 234));

	ShowWindow(hMalformedWindow, SW_SHOW);
	UpdateWindow(hMalformedWindow);
}

//
//  FUNCTION: SortPrompts()
//
//  PURPOSE: Validate and sort the prompts by weight, lexicographically, and remove duplicates.
//
void SortPrompts() {
	// Get length of input text
	int length = GetWindowTextLengthW(hInputEdit);

	if (length > 0) {
		// Allocate buffer for input text
		std::wstring inputText(length + 1, L'\0');
		GetWindowTextW(hInputEdit, inputText.data(), length + 1);
		inputText.resize(length); // Remove extra null terminator

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

		// CLIP BPE token count estimator
		auto countTokens = [](const std::wstring& s) {
			int tokens = 0;
			bool inWord = false;

			for (wchar_t c : s) {
				// Spaces are skipped/merged into text words
				if (c == L' ' || c == L'\t' || c == L'\r' || c == L'\n') {
					inWord = false;
					continue;
				}

				// Alphanumeric groups and underscores form single words
				if (iswalnum(c) || c == L'_') {
					if (!inWord) {
						tokens++;
						inWord = true;
					}
				}
				else {
					// Every bracket, colon, pipe, and weight symbol is its own token
					tokens++;
					inWord = false;
				}
			}
			return tokens;
		};

		auto convertBracketWeight = [&trim](const std::wstring& token) -> std::wstring {
			size_t parenOpen = 0, parenClose = 0;

			while (parenOpen < token.size() && token[parenOpen] == L'(')
				parenOpen++;

			while (parenClose < token.size() && token[token.size() - 1 - parenClose] == L')')
				parenClose++;

			size_t squareOpen = 0, squareClose = 0;

			while (squareOpen < token.size() && token[squareOpen] == L'[')
				squareOpen++;

			while (squareClose < token.size() && token[token.size() - 1 - squareClose] == L']')
				squareClose++;

			std::wstring text;
			double weight = 1.0;

			if (parenOpen > 0 && parenOpen == parenClose) {
				text = trim(token.substr(parenOpen, token.size() - parenOpen - parenClose));
				weight = std::pow(1.1, static_cast<double>(parenOpen));
			}
			else if (squareOpen > 0 && squareOpen == squareClose) {
				text = trim(token.substr(squareOpen, token.size() - squareOpen - squareClose));
				weight = std::pow(1.0 / 1.1, static_cast<double>(squareOpen));
			}
			else {
				return token;
			}

			std::wstringstream ss;
			ss << L"(" << text << L":";
			ss << std::fixed << std::setprecision(3) << weight;
			ss << L")";

			return ss.str();
		};

		auto hasMalformedBrackets = [](const std::wstring& s) {
			int parenCount = 0;
			int squareCount = 0;

			for (wchar_t c : s) {
				if (c == L'(')
					parenCount++;
				else if (c == L')')
					parenCount--;
				else if (c == L'[')
					squareCount++;
				else if (c == L']')
					squareCount--;

				// Closing before opening
				if (parenCount < 0 || squareCount < 0)
					return true;
			}

			return parenCount != 0 || squareCount != 0;
		};

		std::vector<Prompt> prompts;
		std::vector<std::wstring> malformedPrompts;
		std::vector<std::wstring> longPrompts;

		std::wstringstream ss(inputText);
		std::wstring token;

		while (std::getline(ss, token, L',')) {
			token = trim(token);
			if (token.empty())
				continue;

			if (hasMalformedBrackets(token)) {
				malformedPrompts.push_back(token);
			}

			if (countTokens(token) > 75) {
				longPrompts.push_back(token);
			}

			Prompt p{ token, token, 1.0 };

			// (((text))) / [[[text]]] into (text:weight)
			std::wstring converted = convertBracketWeight(token);

			if (converted != token) {
				p.original = converted;
				token = converted;

				size_t colon = converted.rfind(L':');

				p.text = trim(converted.substr(1, colon - 1));

				p.weight = std::stod(
					converted.substr(colon + 1, converted.size() - colon - 2)
				);
			}

			// (text:1.2)
			if (token.size() >= 5 && token.front() == L'(' && token.back() == L')') {
				// Minimum size of 5 characters for (x:y)
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

			prompts.push_back(std::move(p));
		}

		// Malformed brackets
		if (!malformedPrompts.empty()) {
			std::wstring message = L"Error: opening and/or closing brackets missing for the following prompt(s): ";

			for (size_t i = 0; i < malformedPrompts.size(); ++i) {
				if (i)
					message += L", ";

				message += malformedPrompts[i];
			}

			message += L"\r\n";

			ShowWarningWindow(message);
		}

		if (!longPrompts.empty()) {
			std::wstring message = L"Warning: the following prompt(s) exceed 75 tokens: ";

			for (size_t i = 0; i < longPrompts.size(); ++i) {
				if (i)
					message += L", ";

				message += longPrompts[i];
			}

			message += L"\r\n";

			ShowWarningWindow(message);
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


		SetWindowTextW(hOutputDisplay, inputText.c_str());
	}
	else {
		// Clear output if input is empty
		SetWindowTextW(hOutputDisplay, L"");
	}
}

void CopyOutput() {
	int length = GetWindowTextLengthW(hOutputDisplay);
	if(length <= 0)
		return;

	std::wstring text(length + 1, L'\0');
	GetWindowTextW(hOutputDisplay, text.data(), length + 1);

	if (!OpenClipboard(nullptr))
		return;

	EmptyClipboard();

	SIZE_T bytes = (length + 1) * sizeof(wchar_t);

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
	if (hMem) {
		void* pMem = GlobalLock(hMem);
		if (pMem != 0) {
			memcpy(pMem, text.c_str(), bytes);
			GlobalUnlock(hMem);

			SetClipboardData(CF_UNICODETEXT, hMem);
			// Do NOT GlobalFree(hMem); the clipboard owns it now.
		}
	}

	CloseClipboard();
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
		case ID_COPY_OUTPUT_BUTTON:
			CopyOutput();
			break;
		case ID_SORT_PROMPT_BUTTON:
			SortPrompts();
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

		if (dis->CtlID == ID_SORT_PROMPT_BUTTON ||
			dis->CtlID == ID_COPY_OUTPUT_BUTTON) {
			HDC dc = dis->hDC;
			RECT rc = dis->rcItem;

			// background
			FillRect(dc, &rc, CreateSolidBrush(RGB(0, 0, 0)));

			// border
			//FrameRect(dc, &rc, GetSysColorBrush(GRAY_BRUSH));

			// text
			SetBkMode(dc, TRANSPARENT);

			const wchar_t* buttonText = L"";

			switch (dis->CtlID) {
				case ID_SORT_PROMPT_BUTTON:
					buttonText = L"Sort Prompts";
				break;
				case ID_COPY_OUTPUT_BUTTON:
					buttonText = L"Copy Output";
				break;
			}

			DrawTextW(
				dc,
				buttonText,
				-1,
				&rc,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			return TRUE;
		}

		break;
	}
	/*case EM_SETBKGNDCOLOR:
	{
		HDC hdc = (HDC)wParam;
		HWND hEdit = (HWND)lParam;

		if (hEdit == hInputEdit || hEdit == hOutputDisplay) {
			SetTextColor(hdc, RGB(64, 64, 64));
			SetBkColor(hdc, RGB(192, 192, 192));

			return (INT_PTR)g_hTextAreaBgBrush;
		}

		break;
	}*/
	case WM_CTLCOLORSTATIC:
	{
		HDC hdc = (HDC)wParam;
		HWND hCtrl = (HWND)lParam;

		if (hCtrl == hOutputDisplay) {
			SetTextColor(hdc, RGB(64, 64, 64));
			SetBkColor(hdc, RGB(192, 192, 192));
			return (INT_PTR)g_hTextAreaBgBrush;
		}
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		// TODO
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		// Delete GDI object(s) on exit
		DeleteObject(g_hTextAreaBgBrush);

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// TODO (syntax highlighting example, with EM_EXGETSEL and EM_EXSETSEL)
//CHARRANGE cr;
//cr.cpMin = 10;
//cr.cpMax = 11;
//
//SendMessage(
//	hInputEdit,
//	EM_EXSETSEL,
//	0,
//	(LPARAM)&cr);
//
//CHARFORMAT2 cf = {};
//cf.cbSize = sizeof(cf);
//cf.dwMask = CFM_COLOR;
//cf.crTextColor = RGB(0, 255, 0);
//
//SendMessage(
//	hInputEdit,
//	EM_SETCHARFORMAT,
//	SCF_SELECTION,
//	(LPARAM)&cf);