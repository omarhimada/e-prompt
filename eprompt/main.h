#pragma once

#include "Resource.h"

#define MAX_LOADSTRING 100
#define ID_INPUT_EDIT 1001
#define ID_SORT_PROMPT_BUTTON 1002
#define ID_COPY_OUTPUT_BUTTON 1003
#define ID_OUTPUT_EDIT 1004
#define BUTTON_HEIGHT 28

// Global Variables:
HINSTANCE hInst;                                // Current instance
WCHAR szTitle[MAX_LOADSTRING];                  // Title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // Main window class name
HWND hInputEdit;                                // Input
HWND hSortPromptButton;                         // Sort button
HWND hCopyOutputButton;							// Copy button
HWND hOutputDisplay;                            // Output
WNDPROC g_DefaultEditProc;						// 'CTRL+A' selection

// Forward declarations of functions included in this code module:
ATOM                RegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                ResizeControls(HWND hWnd);
void                SortPrompt();

struct Prompt {
	std::wstring original;
	std::wstring text;
	double weight;
};