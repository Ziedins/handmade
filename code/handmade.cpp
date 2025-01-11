#include <windows.h>
#include <stdint.h>
#define internal static 
#define local_persist static 
#define global_variable static 

typedef uint8_t uint8;
typedef uint32_t uint32;

//Global for now
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory; 
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;


internal void 
RenderWeirdGradients(int XOffset, int YOffset)
{
  int Width = BitmapWidth;
  int Height = BitmapHeight;
  
  int Pitch = Width * BytesPerPixel;
  int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;
  BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  uint8 *Row = (uint8 *)BitmapMemory;
  for (int Y = 0; Y < BitmapHeight; ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < BitmapWidth; ++X) {
      uint8 Blue =(X + XOffset); 
      uint8 Green =(Y + YOffset); 
      uint8 Red =(X + XOffset); 
      *Pixel++ = ((Green << 8) | Blue);
    }

    Row += Pitch;
  }
}

internal void Win32ResizeDIBSection(int Width, int Height)
{
  if (BitmapMemory) {
    VirtualFree(BitmapMemory, 0, MEM_RELEASE);
  }
  BitmapWidth = Width;
  BitmapHeight = Height;

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader); 
  BitmapInfo.bmiHeader.biWidth = BitmapWidth;
  BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int Width, int Height)
{
  int WindowWidth = WindowRect->right - WindowRect->left;
  int WindowHeight = WindowRect->bottom - WindowRect->top;
  StretchDIBits(
    DeviceContext,
    /*X, Y, Width, Height,*/
    /*X, Y, Width, Height,*/
    0, 0, BitmapWidth, BitmapHeight,
    0, 0, WindowWidth, WindowHeight,
    BitmapMemory,
    &BitmapInfo,
    DIB_RGB_COLORS,
    SRCCOPY
  );
}

LRESULT CALLBACK 
MainWindowCallback(
  HWND Window,
  UINT Message,
  WPARAM WParam,
  LPARAM LParam)
{
  LRESULT Result = 0;

  switch (Message) {
    case WM_SIZE: 
    {
      RECT ClientRect;
      GetClientRect(Window, &ClientRect);
      int Width = ClientRect.right - ClientRect.left;
      int Height = ClientRect.bottom - ClientRect.top;
      Win32ResizeDIBSection(Width, Height);
      OutputDebugStringA("WM_SIZE\n");
    } break;

    case WM_DESTROY: 
    {
      Running = false;
      OutputDebugStringA("WM_DESTROY\n");
    } break;
          
    case WM_CLOSE: 
    {
      Running = false;
      OutputDebugStringA("WM_CLOSE\n");
    } break;

    case WM_ACTIVATEAPP:
    {
      OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT:
    {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(Window, &Paint);
      int X = Paint.rcPaint.left;
      int Y = Paint.rcPaint.top;
      int Width = Paint.rcPaint.right - Paint.rcPaint.left;
      int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
      RECT ClientRect;
      GetClientRect(Window, &ClientRect);
      Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
      EndPaint(Window, &Paint);
    } break;
    default:
    {
     // OutputDebugStringA("default\n");
      Result = DefWindowProcA(Window, Message, WParam, LParam);
    } break;
  }

  return(Result);
}

int CALLBACK WinMain(
  HINSTANCE Instance,
  HINSTANCE PrevInstance,
  LPSTR     CommandLine,
  int       ShowCode)
{
  WNDCLASS WindowClass = {};

  WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = Instance;
//  WindowClass.hIcon;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";

  if (RegisterClassA(&WindowClass)) {
    HWND Window = 
      CreateWindowExA(
       0,
       WindowClass.lpszClassName,
       "RGB, red + blue is pink, plus green is Fontaines DC",
       WS_OVERLAPPEDWINDOW|WS_VISIBLE,
       CW_USEDEFAULT,
       CW_USEDEFAULT,
       CW_USEDEFAULT,
       CW_USEDEFAULT,
       0,
       0,
       Instance,
       0
      );
     
    if (Window) {
      Running = true;
      int XOffset = 0;
      int YOffset = 0;
      while (Running)
      {
        MSG Message;
    
        while (PeekMessageA(&Message,0,0,0,PM_REMOVE))
        {
          if (Message.message == WM_QUIT) {
            Running = false;
          }
          TranslateMessage(&Message);
          DispatchMessage(&Message);         
        }

        RenderWeirdGradients(XOffset, YOffset);
        HDC DeviceContext = GetDC(Window);
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        int WindowWidth = ClientRect.right - ClientRect.left;
        int WindowHeight = ClientRect.bottom - ClientRect.top;
        Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
        ReleaseDC(Window, DeviceContext);

        ++XOffset;
      }
    }
    else {
      /*TODO Logging*/
    }
  } else {
    //TODO Logging
  }

  return(0);
}

