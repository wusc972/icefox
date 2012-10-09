
#pragma once
//define
#define TRAY_NOTIFY 3001



//
static BOOL InitWindow( HINSTANCE hInstance, int nCmdShow );
void createCtrls(HWND&hwnd, WPARAM wParam);
BOOL CALLBACK WinProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
void onTray(WPARAM wParam,LPARAM lParam);
//
void setIcon(bool disabled ,bool deL = false);
void setTips(const char *tips);

static bool firstTimeToAdd = true;
