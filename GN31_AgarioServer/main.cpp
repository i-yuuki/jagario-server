#define NOMINMAX

#include <algorithm>
#include <WinSock2.h>

#include "server.h"
#include "winsock-error.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

constexpr int SCREEN_WIDTH = 640;
constexpr int SCREEN_HEIGHT = 480;

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int cmdShow){
  constexpr const wchar_t* windowClassName = L"GN31GameServer";
  constexpr const wchar_t* windowTitle = L"さーばー";
  constexpr DWORD windowStyle = WS_OVERLAPPEDWINDOW;

  CoInitialize(nullptr);

  // おなじみ クラス登録
  WNDCLASSW cls = {};
  cls.lpfnWndProc   = WindowProc;
  cls.lpszClassName = windowClassName;
  cls.hInstance     = instance;
  cls.hCursor       = LoadCursor(NULL, IDC_ARROW);
  cls.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1); // ブラシオブジェクト or (HBRUSH)(COLOR_XXX + 1)
  RegisterClassW(&cls);

  // タイトルバーとかを含めたウィンドウサイズを求める
  RECT windowRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
  AdjustWindowRect(&windowRect, windowStyle, false);
  int windowWidth  = windowRect.right - windowRect.left;
  int windowHeight = windowRect.bottom - windowRect.top;
  
  // work area (タスクバーを除いた画面サイズ) の真ん中に配置
  // デスクトップの真ん中だと画面とウィンドウのサイズが近い
  // (たとえば斡旋PCの画面1366x768 & ウィンドウ1280x720)
  // とウィンドウ下端がタスクバーに埋まるため
  RECT workRect;
  SystemParametersInfoW(SPI_GETWORKAREA, 0, &workRect, 0);
  int workWidth  = workRect.right - workRect.left;
  int workHeight = workRect.bottom - workRect.top;
  int windowX =             (workWidth  - windowWidth)  / 2;
  int windowY = std::max(0, (workHeight - windowHeight) / 2);

  HWND window = CreateWindowW(
    windowClassName,
    windowTitle,
    windowStyle,
    windowX,
    windowY,
    windowWidth,
    windowHeight,
    NULL,
    NULL,
    instance,
    NULL
  );
  
  if(!window){
    return 1;
  }
  
  ShowWindow(window, SW_NORMAL);
  UpdateWindow(window);

  MSG msg{};
  while(msg.message != WM_QUIT){
    if(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)){
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }else{
      GameServer::Instance.tick();
      Sleep(50); // 適当
    }
  }

  return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam){
  switch(msg){
    case WM_CREATE:
      GameServer::Instance.init();
      break;
    case WM_DESTROY:
      GameServer::Instance.uninit();
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProc(window, msg, wParam, lParam);
  }
  return 0;
}
