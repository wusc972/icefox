
#define _WIN32_IE 0x0500 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "io.h"
#include "windows.h"
#include <windowsx.h>
#include "CommCtrl.h"
#include <windef.h>
#include <gdiplus.h>

#include "resource.h"
#include "main.h"
//
#include "httpsocket.h"
#include "youconfig.h"
#include "httpserver.h"
#include "systemutil.h"

#include <signal.h>
#include <iostream>
#include <ctime>



using namespace std;

static HWND wnd;
static HINSTANCE hinstance;
static HMENU menu;
static SystemUtil ieSetter;
static bool bAgentOn = false;

const static wchar_t statement[]=
L"YouProxy For Windows (Build 2)\n\
=============================\n\
\n\
* YouProxy 是一款免费软件，欢迎所有人提供BUG信息及建议。\n\
* 请访问 http://code.google.com/p/icefox/ 获取最新信息 。\n\
* 版本信息：20120908 (i386-mingw)\n \
      \n\
      \n\
* 使用方法：右键托盘菜单中，选择启用 YouProxy 。\n \
";

int PASCAL WinMain( HINSTANCE hInstance, //当前实例句柄
		HINSTANCE hPrevInstance, //前一个实例句柄
		LPSTR lpCmdLine, //命令行字符
		int nCmdShow) //窗口显示方式
{
	MSG msg;
	//创建主窗口
	if ( !InitWindow( hInstance, nCmdShow ) )
		return FALSE;
	//进入消息循环
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//程序结束
ieSetter.disableSystemProxy();
setIcon(true,true);
	return msg.wParam;
}

static BOOL InitWindow( HINSTANCE hInstance, int nCmdShow )
{
	WNDCLASS wc; //窗口类结构
	//填充窗口类结构
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(MAINAPP));
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground =(HBRUSH) CreateSolidBrush(RGB(240,240,240));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "youagent";
	//注册窗口类
	RegisterClass( &wc );
	//创建主窗口
        long screenw,screenh;
        screenw=GetSystemMetrics(SM_CXSCREEN);
        screenh=GetSystemMetrics(SM_CYSCREEN);
        long ww= 599 ,hh= 280 ;
	wnd = CreateWindowW(
			L"youagent", //窗口类名称
			L" YouProxy ", //窗口标题
			WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, //窗口风格，定义为普通型
			(screenw-ww)/2, //窗口位置的x坐标
			(screenh-hh)/2, //窗口位置的y坐标
			ww, //窗口的宽度
			hh, //窗口的高度
			NULL, //父窗口句柄
			NULL, //菜单句柄
			hInstance, //应用程序实例句柄
			NULL ); //窗口创建数据指针
	if( !wnd ) return FALSE;
	//
	SendMessage(wnd, WM_SETICON, TRUE, (LPARAM) 
    LoadIcon((HINSTANCE) GetWindowLong(wnd, GWL_HINSTANCE),MAKEINTRESOURCE(MAINAPP)));
	hinstance=hInstance;
	//显示并更新窗口
	ShowWindow( wnd, SW_SHOW);
	
	setIcon(true);
	setTips("YouProxy is DISabled!");
	return TRUE;
}
///消息循环
BOOL CALLBACK WinProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  switch( message )
    {
    case WM_CREATE:
      {
								
	menu=LoadMenu(hinstance, MAKEINTRESOURCE (IDR_MENU1)) ;
	createCtrls(hWnd,wParam);
	//init ie settings
	ieSetter.disableSystemProxy();
	//开启服务
#ifndef __WIN32__
	signal(SIGPIPE, SIG_IGN);
#else
	static WSADATA wsa_data;
	if(WSAStartup((WORD)(1<<8|1), &wsa_data) != 0)
	  exit(1);
#endif
	      
	///
	YouConfig::instance()->loadFromNetwork();
	HttpServer* server = new HttpServer();
	//cout << "Start proxy server at localhost:1998\n"<<endl;
	server->startThread();


	break;
      }
    case  WM_COMMAND:
      {
	switch( LOWORD( wParam))
	  {
	  case IDM_1://enable
	    {
	      if (!bAgentOn)
		{
		  CheckMenuItem( menu, IDM_1, MF_BYCOMMAND | MF_CHECKED); 
		  ///set local ie settings
		  ieSetter.setSystemProxy("127.0.0.1",1998);
		  //flag
		  bAgentOn = true;
		  //tray icon
		  setIcon(false);
setTips("YouProxy is ENabled!");
		}
	      else {
		CheckMenuItem( menu, IDM_1, MF_BYCOMMAND | MF_UNCHECKED); 
		ieSetter.disableSystemProxy();
		bAgentOn = false;
		setIcon(true);
setTips("YouProxy is DISabled!");
	      }
	      break;
	    }
	  case IDM_2://close
	    {
	      DestroyWindow(wnd);
	      break;
	    }
	  case IDBT:
	    {
	      ShowWindow(wnd,SW_HIDE);
	      break;
	    }
	    
	  }
	break;
      }
    case TRAY_NOTIFY:
      {
	onTray(wParam,lParam);
	break;
      }
		
    case WM_DESTROY://退出消息
      {
				
	PostQuitMessage( 0 );//调用退出函数
	break;
      }
      /*
    default:
      if (message == WM_TASKBARCREATED)
	{
	  firstTimeToAdd = true;
	  setIcon(!bAgentOn);
	  }*/
    }
  //调用缺省消息处理过程
  return DefWindowProc(hWnd, message, wParam, lParam);
} 

void createCtrls(HWND&hwnd,WPARAM wParam)
{

	HFONT hfont0 = CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, 1, 400, 0, 0, 0, ("Ms Shell Dlg"));
	HWND hCtrl0_0 =CreateWindowW( L"BUTTON",L"隐藏",
				      WS_VISIBLE|WS_CHILD, 486, 205, 99, 36,
					hwnd,(HMENU)IDBT,hinstance,NULL);
	
	SendMessage(hCtrl0_0, WM_SETFONT, (WPARAM)hfont0, TRUE);
	HWND hCtrl0_1 =CreateWindowW( L"STATIC",statement,
			       WS_VISIBLE | WS_CHILD | WS_GROUP | SS_LEFT,11, 20, 399, 184,
			       hwnd, NULL,hinstance,NULL);
	SendMessage(hCtrl0_1, WM_SETFONT, (WPARAM)hfont0, TRUE);

}


void onTray(WPARAM wParam,LPARAM lParam)
{
  UINT uMouseMsg = (UINT) lParam;

  switch(uMouseMsg){

  case WM_LBUTTONDBLCLK:
    {
      //show or hide the dialog
      if(IsWindowVisible(wnd))
	ShowWindow(wnd,SW_HIDE);
      else
	ShowWindow(wnd,SW_NORMAL);
      SetForegroundWindow(wnd);
      break;
    }
  case WM_RBUTTONDOWN:
    {
      HMENU popmenu=GetSubMenu(menu,0);
      POINT point;
      GetCursorPos(&point);
      //this can makesure menu get focus right now
      SetForegroundWindow(wnd);
      TrackPopupMenu (popmenu, TPM_RIGHTBUTTON, point.x, point.y, 0, wnd, NULL) ;
      break;

    }
  default: 
    break;
  }
}



void setIcon(bool disabled , bool deL )
{
	// set NOTIFYCONDATA structure	
	NOTIFYICONDATA tnid;

	tnid.cbSize = sizeof(NOTIFYICONDATA); 
	tnid.hWnd = wnd; 
	tnid.uID = 0x1; //Resourcename
	tnid.uFlags = NIF_MESSAGE |NIF_ICON | NIF_TIP; //
	tnid.uCallbackMessage = TRAY_NOTIFY;//user message 
	
	HICON iconnew;
	if(!disabled)
	  iconnew = LoadIcon(hinstance, MAKEINTRESOURCE(MAINAPP));
	else
	  iconnew = LoadIcon(hinstance, MAKEINTRESOURCE(AGENTOFF));
	tnid.hIcon = iconnew; //handle of the created icon

	//copy the string to the NOTIFYCONDATA structure
	lstrcpyn(tnid.szTip, "YouProxy for Windows", sizeof(tnid.szTip));

	if (deL)
	{
		Shell_NotifyIcon(NIM_DELETE,&tnid);
		return;
	}

	
	if (firstTimeToAdd){
	// add the icon to the tray
	  Shell_NotifyIcon(NIM_ADD, &tnid); 
	  firstTimeToAdd = false;
	}
	else{
	  Shell_NotifyIcon(NIM_MODIFY, &tnid);
	}
	  
	// free icon 
	DestroyIcon(iconnew); 
}

void setTips(const char* tips)
{
// set NOTIFYCONDATA structure	
	NOTIFYICONDATA m_tnd;

	m_tnd.cbSize=sizeof(NOTIFYICONDATA);
	m_tnd.uFlags = NIF_INFO;
	m_tnd.uVersion = NOTIFYICON_VERSION;
	m_tnd.uTimeout = 1000;
	m_tnd.dwInfoFlags = NIIF_INFO;
	wsprintf( m_tnd.szInfoTitle, "%s"," YouProxy " );
	wsprintf( m_tnd.szInfo,"%s",      tips     );
	//wcscpy_s( m_tnd.szTip,       _T("tip")       );
	//SetTimer(1, 1000, NULL);
	Shell_NotifyIcon( NIM_MODIFY, &m_tnd );
}

