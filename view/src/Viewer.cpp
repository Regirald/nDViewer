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
}

// Update scene rotation based on simple yaw/pitch driven by left mouse drag.
void Viewer::handleMouseMessage(unsigned int msg, unsigned long long wParam, long long lParam)
{
    const int x = GET_X_LPARAM(lParam);
    const int y = GET_Y_LPARAM(lParam);

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
    case WM_MOUSEMOVE:
        if (m_leftDown)
        {
            const int dx = x - m_lastX;
            const int dy = y - m_lastY;

            m_lastX = x;
            m_lastY = y;

            // Sensitivity factor for rotation.
            const float sensitivity = 0.25f;
            m_yaw   += dx * sensitivity;
            m_pitch += dy * sensitivity;

            // Clamp pitch to avoid gimbal lock.
            //if (m_pitch > 89.0f)  m_pitch = 89.0f;
            //if (m_pitch < -89.0f) m_pitch = -89.0f;

            const float degToRad = 3.1415926535f / 180.0f;
            const float cy = cosf(m_yaw * degToRad);
            const float sy = sinf(m_yaw * degToRad);
            const float cp = cosf(m_pitch * degToRad);
            const float sp = sinf(m_pitch * degToRad);

            // Row-major 4x4 rotation matrix R = R_yaw * R_pitch.
            Mat4 rot;
            rot.m[0] = cy;  rot.m[1] = 0.0f; rot.m[2]  = sy;  rot.m[3]  = 0.0f;
            rot.m[4] = sy*sp; rot.m[5] = cp;  rot.m[6]  = -cy*sp; rot.m[7]  = 0.0f;
            rot.m[8] = -sy*cp; rot.m[9] = sp;  rot.m[10] = cy*cp; rot.m[11] = 0.0f;
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
        glClearColor(0.f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(m_sceneMatrix.m);
        glEnable(GL_DEPTH_TEST);

		m_render.renderScene();

        glBegin(GL_QUADS);

        // +X face (red)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f( 1.0f, -1.0f, -1.0f);
        glVertex3f( 1.0f,  1.0f, -1.0f);
        glVertex3f( 1.0f,  1.0f,  1.0f);
        glVertex3f( 1.0f, -1.0f,  1.0f);

        // -X face (green)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f,  1.0f);
        glVertex3f(-1.0f,  1.0f,  1.0f);
        glVertex3f(-1.0f,  1.0f, -1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

        // +Y face (blue)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f,  1.0f, -1.0f);
        glVertex3f( 1.0f,  1.0f, -1.0f);
        glVertex3f( 1.0f,  1.0f,  1.0f);
        glVertex3f(-1.0f,  1.0f,  1.0f);

        // -Y face (yellow)
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f,  1.0f);
        glVertex3f( 1.0f, -1.0f,  1.0f);
        glVertex3f( 1.0f, -1.0f, -1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

        // +Z face (cyan)
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f,  1.0f);
        glVertex3f(-1.0f,  1.0f,  1.0f);
        glVertex3f( 1.0f,  1.0f,  1.0f);
        glVertex3f( 1.0f, -1.0f,  1.0f);

        // -Z face (magenta)
        glColor3f(1.0f, 0.0f, 1.0f);
        glVertex3f( 1.0f, -1.0f, -1.0f);
        glVertex3f( 1.0f,  1.0f, -1.0f);
        glVertex3f(-1.0f,  1.0f, -1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

        glEnd();

        // Call user render callback if set.
        if (m_renderCallback)
        {
            m_renderCallback(width, height, m_sceneMatrix);
        }

        

        SwapBuffers(g_hDC);
    }

    ShutdownOpenGL();
}

void Viewer::setMesh(Mesh* mesh)
{
    m_render.
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