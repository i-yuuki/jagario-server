#define NOMINMAX

#include <algorithm>
#include <chrono>
#include <string>
#include <WinSock2.h>

#include "server.h"
#include "winsock-error.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

constexpr int SCREEN_WIDTH = 640;
constexpr int SCREEN_HEIGHT = 640;

static HDC g_memCtx;
static HBITMAP g_bitmap;
static HGDIOBJ g_brush;

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
void draw();
void drawText(HDC ctx, int x, int y, const std::string& str);
std::wstring widen(const std::string& str);

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

  using clock = std::chrono::high_resolution_clock;
  clock::time_point timeNextTick = clock::now();

  MSG msg{};
  while(msg.message != WM_QUIT){
    if(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)){
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }else{
      auto now = clock::now();
      if(now >= timeNextTick){
        GameServer::Instance.tick();
        draw();
        InvalidateRect(window, NULL, false);
        timeNextTick += std::chrono::milliseconds(50);
      }else{
        Sleep(std::chrono::duration_cast<std::chrono::milliseconds>(timeNextTick - now).count());
      }
    }
  }

  return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam){
  switch(msg){
    case WM_CREATE:
    {
      GameServer::Instance.init();
      HDC ctx = GetDC(window);
      g_memCtx = CreateCompatibleDC(ctx);
      g_bitmap = CreateCompatibleBitmap(ctx, SCREEN_WIDTH, SCREEN_HEIGHT);
      g_brush = GetStockObject(NULL_BRUSH);
      SelectObject(g_memCtx, g_bitmap);
      SelectObject(g_memCtx, g_brush);
      SetBkMode(g_memCtx, TRANSPARENT);
      SetTextColor(g_memCtx, RGB(0, 0, 0));
      ReleaseDC(window, ctx);
      break;
    }
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC ctx = BeginPaint(window, &ps);
      BitBlt(ctx, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_memCtx, 0, 0, SRCCOPY);
      EndPaint(window, &ps);
      break;
    }
    case WM_DESTROY:
      GameServer::Instance.uninit();
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProc(window, msg, wParam, lParam);
  }
  return 0;
}

void draw(){
  PatBlt(g_memCtx, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITENESS);
  drawText(g_memCtx, 4, 4, std::to_string(GameServer::Instance.getPlayers().size()) + u8"人がプレイ中");
  
  // TODO GameServer.fieldSizeを使う
  float scale = SCREEN_WIDTH / 2000.0f;
  for(auto it : GameServer::Instance.getPlayers()){
    auto& p = it.second;
    int x = static_cast<int>(p.posX * scale);
    int y = static_cast<int>(p.posY * scale);
    int r = std::max(static_cast<int>(p.size * scale / 2), 1);
    Ellipse(g_memCtx, x - r, y - r, x + r, y + r);
    drawText(g_memCtx, x, y, std::string(p.name).append(u8" (").append(std::to_string(p.size)).append(u8")"));
  }
}

void drawText(HDC ctx, int x, int y, const std::string& str){
  auto wide = widen(str);
  TextOutW(ctx, x, y, wide.c_str(), wide.length());
}

std::wstring widen(const std::string& str){
  int wideLen = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
  std::wstring wide;
  wide.resize(wideLen);
  MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), &wide.at(0), wideLen);
  return wide;
}
