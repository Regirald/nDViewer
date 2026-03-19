#include <windows.h>
#include <gl/GL.h>
#include <string>
#include "Viewer.h"

// Link against opengl32
#pragma comment(lib, "opengl32.lib")

// Global (for this translation unit) handles.
static HDC   g_hDC   = nullptr;
static HGLRC g_hGLRC = nullptr;

// Forward declarations.
static bool InitOpenGL(HWND hWnd);
static void ShutdownOpenGL();

// Simple window procedure that handles paint/resize/close.
static LRESULT CALLBACK SimpleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

Viewer::Viewer(const std::string& title, int width, int height)
    : m_title(title)
    , m_width(width)
    , m_height(height)
{
}

void Viewer::run()
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    const wchar_t* className = L"NDViewerWindowClass";

    // Register window class (ignore failure if already registered).
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
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        m_width, m_height,
        nullptr, nullptr,
        hInstance,
        nullptr);

    if (!hWnd)
        return;

    // Initialize OpenGL context.
    if (!InitOpenGL(hWnd))
    {
        DestroyWindow(hWnd);
        return;
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    // Main loop: pump messages and render when idle.
    MSG msg = {};
    bool running = true;
    while (running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running)
            break;

        // Get current client size.
        RECT rect;
        GetClientRect(hWnd, &rect);
        int width  = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        if (height <= 0) height = 1;

        // Setup basic viewport and clear.
        glViewport(0, 0, width, height);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth test for 3D if needed.
        glEnable(GL_DEPTH_TEST);

        // Call user render callback if set.
        if (m_renderCallback)
        {
            m_renderCallback(width, height);
        }
        else
        {
            // Simple placeholder: draw a colored quad using fixed function.
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glBegin(GL_QUADS);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2f(-0.5f, -0.5f);
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex2f( 0.5f, -0.5f);
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex2f( 0.5f,  0.5f);
            glColor3f(1.0f, 1.0f, 0.0f);
            glVertex2f(-0.5f,  0.5f);
            glEnd();
        }

        SwapBuffers(g_hDC);
    }

    ShutdownOpenGL();
}

// --- OpenGL setup/teardown ---

static bool InitOpenGL(HWND hWnd)
{
    g_hDC = GetDC(hWnd);
    if (!g_hDC)
        return false;

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits= 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    if (!pixelFormat)
        return false;

    if (!SetPixelFormat(g_hDC, pixelFormat, &pfd))
        return false;

    g_hGLRC = wglCreateContext(g_hDC);
    if (!g_hGLRC)
        return false;

    if (!wglMakeCurrent(g_hDC, g_hGLRC))
        return false;

    return true;
}

static void ShutdownOpenGL()
{
    if (g_hGLRC)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(g_hGLRC);
        g_hGLRC = nullptr;
    }

    if (g_hDC)
    {
        ReleaseDC(WindowFromDC(g_hDC), g_hDC);
        g_hDC = nullptr;
    }
}