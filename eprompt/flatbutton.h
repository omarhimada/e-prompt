#pragma once

#include <windows.h>
#include <string>

struct FlatButtonData {
    COLORREF background = RGB(245, 245, 245);
    COLORREF textColor = RGB(0, 0, 0);

    COLORREF hoverBackground = RGB(235, 235, 235);
    COLORREF pressedBackground = RGB(220, 220, 220);

    HFONT font = nullptr;

    bool hovered = false;
    bool pressed = false;
};

BOOL RegisterFlatButton(HINSTANCE hInstance);

HWND CreateFlatButton(
    HWND parent,
    int id,
    const wchar_t* text,
    int x,
    int y,
    int width,
    int height);

void FlatButton_SetBackground(HWND hwnd, COLORREF color);
void FlatButton_SetHoverBackground(HWND hwnd, COLORREF color);
void FlatButton_SetPressedBackground(HWND hwnd, COLORREF color);
void FlatButton_SetTextColor(HWND hwnd, COLORREF color);
void FlatButton_SetFont(HWND hwnd, HFONT font);