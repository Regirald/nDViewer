#include <windows.h>
#include "Viewer.h"

// Simple window procedure that closes on WM_DESTROY.
static LRESULT CALLBACK SimpleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

Viewer::Viewer(const std::string& title, int width, int height)
    : m_title(title), m_width(width), m_height(height)
{
}

void Viewer::run()
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    const wchar_t* className = L"NDViewerWindowClass";

    // Register window class once per process.
    WNDCLASSW wc = {};
    wc.lpfnWndProc   = SimpleWndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = className;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    // Convert title to wide string.
    std::wstring wtitle(m_title.begin(), m_title.end());

    // Create the window.
    HWND hWnd = CreateWindowW(
        className,
        wtitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        m_width, m_height,
        nullptr, nullptr,
        hInstance,
        nullptr);

    if (!hWnd)
        return;

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    // Basic message loop.
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}