#include <windows.h>
#include <gl/GL.h>
#include <windowsx.h> 
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

// Store a pointer to the Viewer instance in the GWLP_USERDATA so we can
// forward mouse messages to it.
static LRESULT CALLBACK SimpleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Viewer* viewer = reinterpret_cast<Viewer*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (msg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
        if (viewer)
        {
            viewer->handleMouseMessage(msg, wParam, lParam);
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

Viewer::Viewer(const std::string& title, int width, int height)
    : m_title(title)
    , m_width(width)
    , m_height(height)
{
	m_render.setBackground();
}

// Update scene rotation based on simple yaw/pitch driven by left mouse drag.
void Viewer::handleMouseMessage(unsigned int msg, unsigned long long wParam, long long lParam)
{
    const int x = GET_X_LPARAM(lParam);
    const int y = -GET_Y_LPARAM(lParam);

    switch (msg)
    {
    case WM_LBUTTONDOWN:
        m_leftDown = true;
        m_lastX = x;
        m_lastY = y;
        break;
    case WM_LBUTTONUP:
        m_leftDown = false;
        break;
    case WM_RBUTTONDOWN:
        m_rightDown = true;
        m_lastX = x;
        m_lastY = y;
        break;
    case WM_RBUTTONUP:
        m_rightDown = false;
        break;
    case WM_MOUSEMOVE:
        if (m_leftDown || m_rightDown)
        {
            const int dx = x - m_lastX;
            const int dy = y - m_lastY;

            m_lastX = x;
            m_lastY = y;

            if (m_leftDown)
            {
                // Sensitivity factor for rotation.
                const float sensitivity = 0.25f;
                m_yaw   += dx * sensitivity;
                m_pitch += dy * sensitivity;
            }
            else if (m_rightDown)
            {
                const float zoomSensitivity = 0.01f;
                m_zoom += dy * zoomSensitivity;
                if (m_zoom < 0.01f) m_zoom = 0.01f;
            }

            // Clamp pitch to avoid gimbal lock.
            //if (m_pitch > 89.0f)  m_pitch = 89.0f;
            //if (m_pitch < -89.0f) m_pitch = -89.0f;

            const float degToRad = 3.1415926535f / 180.0f;
            const float cy = cosf(m_yaw * degToRad);
            const float sy = sinf(m_yaw * degToRad);
            const float cp = cosf(m_pitch * degToRad);
            const float sp = sinf(m_pitch * degToRad);

            // Row-major 4x4 rotation matrix R = R_yaw * R_pitch * Zoom
            Mat4 rot;
            rot.m[0] = cy * m_zoom;  rot.m[1] = 0.0f; rot.m[2]  = sy * m_zoom;  rot.m[3]  = 0.0f;
            rot.m[4] = sy*sp * m_zoom; rot.m[5] = cp * m_zoom;  rot.m[6]  = -cy*sp * m_zoom; rot.m[7]  = 0.0f;
            rot.m[8] = -sy*cp * m_zoom; rot.m[9] = sp * m_zoom;  rot.m[10] = cy*cp * m_zoom; rot.m[11] = 0.0f;
            rot.m[12] = 0.0f; rot.m[13] = 0.0f; rot.m[14] = 0.0f; rot.m[15] = 1.0f;

            m_sceneMatrix = rot;
        }
        break;
    default:
        break;
    }
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

    // Associate this Viewer instance with the window for input handling.
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

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

    // High-resolution timer for FPS measurement.
    LARGE_INTEGER perfFreq = {};
    LARGE_INTEGER lastTime = {};
    QueryPerformanceFrequency(&perfFreq);
    QueryPerformanceCounter(&lastTime);

    double accumTime = 0.0;
    int    frameCount = 0;

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

        // Setup basic viewport and clear depth (background renderer clears color).
        glViewport(0, 0, width, height);
        glClear(GL_DEPTH_BUFFER_BIT);



        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(m_sceneMatrix.m);
        glEnable(GL_DEPTH_TEST);


		m_render.renderScene();



        SwapBuffers(g_hDC);

        // --- FPS measurement and simple on-screen text rendering ---
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        double delta = static_cast<double>(now.QuadPart - lastTime.QuadPart) /
                       static_cast<double>(perfFreq.QuadPart);
        lastTime = now;

        accumTime += delta;
        ++frameCount;

        // Update FPS once per second to avoid flicker.
        if (accumTime >= 1.0)
        {
            m_fps = static_cast<float>(frameCount / accumTime);
            accumTime = 0.0;
            frameCount = 0;
        }

        // Draw FPS in the top-left corner using GDI over the backbuffer.
        if (m_fps > 0.0f)
        {
            // Get a window DC; for simplicity we use a short-lived HDC here.
            HDC hdc = GetDC(hWnd);
            if (hdc)
            {
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(255, 255, 255));

                char buffer[64];
                sprintf_s(buffer, sizeof(buffer), "FPS: %.1f", m_fps);

                TextOutA(hdc, 8, 8, buffer, static_cast<int>(strlen(buffer)));

                ReleaseDC(hWnd, hdc);
            }
        }
    }

    ShutdownOpenGL();
}

void Viewer::setMesh(Mesh<2>* mesh)
{
	m_render.setMesh(mesh);
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