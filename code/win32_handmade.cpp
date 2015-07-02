#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

// TODO(Robert): this is a global for now.
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void Win32ResizeDIBSection(int Width, int Height) {
	// TODO(Robert): Bulletproof this.
	// Maybe don't free first, free after, then free first if that fails.

	if (BitmapHandle) {
		DeleteObject(BitmapHandle);
	}
	
	if (!BitmapDeviceContext) {
		// TODO(Robert): Should we recreate these under special circumstances
		BitmapDeviceContext = CreateCompatibleDC(0);
	}

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	// TODO(Robert): maybe we can just allocate this ourselves
	BitmapHandle = CreateDIBSection(
		BitmapDeviceContext,
		&BitmapInfo,
		DIB_RGB_COLORS,
		&BitmapMemory,
		0, 0);
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height) {
	StretchDIBits(DeviceContext, X, Y, Width, Height,
								 X, Y, Width, Height,
								 BitmapMemory,
								 &BitmapInfo,
								 DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK MainWindowCallback(HWND Window,
									UINT Message,
									WPARAM WParam,
									LPARAM LParam) {
	LRESULT Result = 0;

	switch (Message) {
		case WM_SIZE:
		{
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int Width = ClientRect.right - ClientRect.left;
			int Height = ClientRect.bottom - ClientRect.top;
			Win32ResizeDIBSection(Width, Height);
		} break;
		case WM_CLOSE:
		{
			// TODO(Robert): Handle this with a message to the user?
			Running = false;
		} break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP");
		} break;
		case WM_DESTROY:
		{
			// TODO(Robert): Handle this as an error - recreate window?
			Running = false;
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
			EndPaint(Window, &Paint);
		} break;

		default:
		{
			//OutputDebugStringA("default");
			Result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}

	return(Result);
}

int CALLBACK WinMain(HINSTANCE Instance,
					 HINSTANCE PrevInstance,
					 LPSTR CommandLine,
					 int ShowCode) {
	WNDCLASS WindowClass = {};

	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.hInstance = Instance;
	//WindowClass.hIcon = ;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&WindowClass)) {
		HWND WindowHandle = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0);
		if (WindowHandle) {
			Running = true;
			while (Running) {
				MSG Message;
				BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
				if (MessageResult > 0) {
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				} else {
					break;
				}
			}
		} else {
			// TODO(Robert): Logging
		}
	} else {
		// TODO(Robert): Logging
	}

	return(0);
}
