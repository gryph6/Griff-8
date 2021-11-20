// Griff-8.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Griff-8.h"
#include "Chip8.h"

#include <iostream>

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

template<class Interface>
inline void SafeRelease(
    Interface** ppInterfaceToRelease
)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}


#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

Chip8 chip8;

HWND m_hwnd;
ID2D1Factory* m_pDirect2dFactory;
ID2D1HwndRenderTarget* m_pRenderTarget;
ID2D1SolidColorBrush* m_pForegroundBrush;
ID2D1SolidColorBrush* m_pBackgroundBrush;

void CheckerboardDemo(D2D1_RECT_F rect, int row, int col) {
	// Draw checkerboard
	if (row % 2 == 0) {
		if (col % 2 == 0) {
			m_pRenderTarget->FillRectangle(&rect, m_pForegroundBrush);
		}
		else {
			m_pRenderTarget->FillRectangle(&rect, m_pBackgroundBrush);
		}
	}
	else {
		if (col % 2 == 0) {
			m_pRenderTarget->FillRectangle(&rect, m_pBackgroundBrush);
		}
		else {
			m_pRenderTarget->FillRectangle(&rect, m_pForegroundBrush);
		}
	}
}

void Draw(unsigned char* graphics) {
    m_pRenderTarget->BeginDraw();

    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::DarkRed));

    D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

	float pixel_width = rtSize.width / 64.0f;
	float pixel_height = rtSize.height / 32.0f;

    for (int i = 0; i < 64 * 32; ++i) {
        if (graphics[i]) {
			int row = i % 64;
			int col = i / 64;

			D2D1_RECT_F pixel = D2D1::RectF(
				row * pixel_width,
				col * pixel_height,
				row * pixel_width + pixel_width,
				col * pixel_height + pixel_height 
			);

            m_pRenderTarget->DrawRectangle(&pixel, m_pForegroundBrush);
            m_pRenderTarget->FillRectangle(&pixel, m_pForegroundBrush);
        } 
    }

    m_pRenderTarget->EndDraw();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GRIFF8, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRIFF8));

    // Setup Graphics
    if (FAILED(CoInitialize(nullptr))) {
        return -1;
    }

	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
    RECT rc;
    GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top
    );

    hr = m_pDirect2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size),
        &m_pRenderTarget
    );

    hr = m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::LightPink),
        &m_pForegroundBrush
    );

    hr = m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::DarkRed),
        &m_pBackgroundBrush
    );

    chip8.initialize();
    chip8.loadProgram("C:\\Users\\griff\\Downloads\\test_opcode.ch8");
    
	MSG msg;
    bool running = true;

    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER end;

    double chip8_freq = 1 / 60.0;
    double cumulative_time = 0.0;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		chip8.emulateCycle();

		if (chip8.shouldDraw()) {
			Draw(chip8.getGraphics());
		}

        QueryPerformanceCounter(&end);

        long long frame_ticks = end.QuadPart - start.QuadPart;
        start = end;

        double frame_time = 1.0 / frequency.QuadPart * frame_ticks;

        cumulative_time += frame_time;
        if (cumulative_time >= chip8_freq) {
			chip8.updateTimers();
            cumulative_time = cumulative_time - chip8_freq;
        }
    }

    CoUninitialize();

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GRIFF8));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   m_hwnd = CreateWindowW(
       szWindowClass, 
       szTitle, 
       (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX ),
       CW_USEDEFAULT, 
       0, 
       1280, 
       640, 
       nullptr, 
       nullptr, 
       hInstance, 
       nullptr);

   if (!m_hwnd)
   {
      return FALSE;
   }

   ShowWindow(m_hwnd, nCmdShow);
   UpdateWindow(m_hwnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_KEYDOWN: {
        switch (wParam) {
        case 0x5A:
            chip8.setKey(0);
        case 0x58:
            chip8.setKey(1);
        case 0x43:
            chip8.setKey(2);
        case 0x56:
            chip8.setKey(3);
        case 0x41:
            chip8.setKey(4);
        case 0x53:
            chip8.setKey(5);
        case 0x44:
            chip8.setKey(6);
        case 0x46:
            chip8.setKey(7);
        case 0x51:
            chip8.setKey(8);
        case 0x57:
            chip8.setKey(9);
        case 0x45:
            chip8.setKey(10);
        case 0x52:
            chip8.setKey(11);
        case 0x31:
			chip8.setKey(12);
            break;
        case 0x32:
			chip8.setKey(13);
            break;
        case 0x33:
			chip8.setKey(14);
            break;
        case 0x34:
			chip8.setKey(15);
            break;
        }
    }
    case WM_KEYUP: {
        switch (wParam) {
        case 0x5A:
            chip8.unsetKey(0);
        case 0x58:
            chip8.unsetKey(1);
        case 0x43:
            chip8.unsetKey(2);
        case 0x56:
            chip8.unsetKey(3);
        case 0x41:
            chip8.unsetKey(4);
        case 0x53:
            chip8.unsetKey(5);
        case 0x44:
            chip8.unsetKey(6);
        case 0x46:
            chip8.unsetKey(7);
        case 0x51:
            chip8.unsetKey(8);
        case 0x57:
            chip8.unsetKey(9);
        case 0x45:
            chip8.unsetKey(10);
        case 0x52:
            chip8.unsetKey(11);
        case 0x31:
			chip8.unsetKey(12);
            break;
        case 0x32:
			chip8.unsetKey(13);
            break;
        case 0x33:
			chip8.unsetKey(14);
            break;
        case 0x34:
			chip8.unsetKey(15);
            break;
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

