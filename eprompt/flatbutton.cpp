#include "FlatButton.h"

LRESULT CALLBACK FlatButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL RegisterFlatButton(HINSTANCE hInstance) {
	WNDCLASS wc = {};

	wc.lpfnWndProc = FlatButtonProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"FlatButton";
	wc.hCursor = LoadCursor(nullptr, IDC_HAND);
	wc.hbrBackground = nullptr;

	return RegisterClass(&wc);
}

HWND CreateFlatButton(
	HWND parent,
	int id,
	const wchar_t* text,
	int x,
	int y,
	int width,
	int height) {
	HWND hwnd = CreateWindowEx(
		0,
		L"FlatButton",
		text,
		WS_CHILD | WS_VISIBLE,
		x,
		y,
		width,
		height,
		parent,
		(HMENU)id,
		GetModuleHandle(nullptr),
		nullptr);

	return hwnd;
}

static FlatButtonData* GetData(HWND hwnd) {
	return (FlatButtonData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

LRESULT CALLBACK FlatButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	FlatButtonData* data = GetData(hwnd);

	switch (msg) {
		case WM_CREATE:
			data = new FlatButtonData;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
			return 0;

		case WM_DESTROY:
			delete data;
			return 0;

		case WM_SETFONT:
			data->font = (HFONT)wParam;
			InvalidateRect(hwnd, nullptr, TRUE);
			return 0;

		case WM_MOUSEMOVE:
			if (!data->hovered) {
				data->hovered = true;

				TRACKMOUSEEVENT tme = {};
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;

				TrackMouseEvent(&tme);

				InvalidateRect(hwnd, nullptr, TRUE);
			}
			return 0;

		case WM_MOUSELEAVE:
			data->hovered = false;
			data->pressed = false;
			InvalidateRect(hwnd, nullptr, TRUE);
			return 0;

		case WM_LBUTTONDOWN:
			data->pressed = true;
			SetCapture(hwnd);
			InvalidateRect(hwnd, nullptr, TRUE);
			return 0;

		case WM_LBUTTONUP:
		{
			bool click = data->pressed;
			data->pressed = false;
			ReleaseCapture();
			InvalidateRect(hwnd, nullptr, TRUE);

			if (click) {
				SendMessage(
					GetParent(hwnd),
					WM_COMMAND,
					MAKEWPARAM(GetDlgCtrlID(hwnd), BN_CLICKED),
					(LPARAM)hwnd);
			}
			return 0;
		}
		case WM_PAINT:
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hwnd, &ps);

			RECT rc;
			GetClientRect(hwnd, &rc);

			COLORREF color = data->background;

			if (data->pressed)
				color = data->pressedBackground;
			else if (data->hovered)
				color = data->hoverBackground;

			HBRUSH brush = CreateSolidBrush(color);

			FillRect(dc, &rc, brush);

			DeleteObject(brush);

			SetBkMode(dc, TRANSPARENT);
			SetTextColor(dc, data->textColor);

			if (data->font)
				SelectObject(dc, data->font);

			wchar_t text[256];

			GetWindowText(hwnd, text, 256);

			DrawText(
				dc,
				text,
				-1,
				&rc,
				DT_CENTER |
				DT_VCENTER |
				DT_SINGLELINE);

			EndPaint(hwnd, &ps);
			return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void FlatButton_SetBackground(HWND hwnd, COLORREF color) {
	GetData(hwnd)->background = color;
	InvalidateRect(hwnd, nullptr, TRUE);
}

void FlatButton_SetHoverBackground(HWND hwnd, COLORREF color) {
	FlatButtonData* data = GetData(hwnd);

	if (data) {
		data->hoverBackground = color;
		InvalidateRect(hwnd, nullptr, TRUE);
	}
}

void FlatButton_SetPressedBackground(HWND hwnd, COLORREF color) {
	FlatButtonData* data = GetData(hwnd);

	if (data) {
		data->pressedBackground = color;
		InvalidateRect(hwnd, nullptr, TRUE);
	}
}

void FlatButton_SetTextColor(HWND hwnd, COLORREF color) {
	GetData(hwnd)->textColor = color;
	InvalidateRect(hwnd, nullptr, TRUE);
}

void FlatButton_SetFont(HWND hwnd, HFONT font) {
	SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
}