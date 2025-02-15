#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#define internal static 
#define local_persist static 
#define global_variable static 

typedef uint8_t uint8;
typedef uint32_t uint32;

struct win32_offscreen_buffer
{
  BITMAPINFO Info;
  void *Memory; 
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

//Global for now
global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer; 

struct win32_window_dimension {
  int Width;
  int Height;
};

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
  win32_window_dimension Result;
  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;
  
  return Result;
}

internal void 
RenderWeirdGradients(win32_offscreen_buffer Buffer,int XOffset, int YOffset)
{

  uint8 *Row = (uint8 *)Buffer.Memory;
  for (int Y = 0; Y < Buffer.Height; ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < Buffer.Width; ++X) {
      uint8 Blue =(X + XOffset); 
      uint8 Green =(Y + YOffset); 
      uint8 Red =(X); 
      *Pixel++ = ((Green << 8) | Blue | Red << 16);
    }

    Row += Buffer.Pitch;
  }
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
  if (Buffer->Memory) {
    VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
  }
  Buffer->Width = Width;
  Buffer->Height = Height;
  Buffer->BytesPerPixel = 4;

  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader); 
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;
  int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(
    win32_offscreen_buffer Buffer,
    HDC DeviceContext,
    int WindowWidth,
    int WindowHeight
) {
  StretchDIBits(
    DeviceContext,
    /*X, Y, Width, Height,*/
    /*X, Y, Width, Height,*/
    0, 0, WindowWidth, WindowHeight,
    0, 0, Buffer.Width, Buffer.Height,
    Buffer.Memory,
    &Buffer.Info,
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
      win32_window_dimension WindowDimension = Win32GetWindowDimension(Window);
      RECT ClientRect;
      GetClientRect(Window, &ClientRect);

      Win32DisplayBufferInWindow(GlobalBackBuffer, DeviceContext, WindowDimension.Width, WindowDimension.Height);
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
  Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

  WindowClass.style = CS_HREDRAW|CS_VREDRAW;
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
      win32_window_dimension WindowDimension = Win32GetWindowDimension(Window);

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

        for (
           int ControllerIndex = 0;
           ControllerIndex < XUSER_MAX_COUNT;
           ++ControllerIndex
        ) {

          XINPUT_STATE ControllerState;
          /*if(XInputGetState(ControllerIndex,&ControllerState) == ERROR_SUCCESS)*/
          /*{*/
          /**/
          /*} else {*/
          /**/
          /*}*/

        }

        RenderWeirdGradients(GlobalBackBuffer, XOffset, YOffset);

        HDC DeviceContext = GetDC(Window);

         win32_window_dimension WindowDimension = Win32GetWindowDimension(Window);
        Win32DisplayBufferInWindow(GlobalBackBuffer, DeviceContext, WindowDimension.Width, WindowDimension.Height);
        ReleaseDC(Window, DeviceContext);

        YOffset += 1;
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

