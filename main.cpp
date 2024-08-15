#include <windows.h>
#include <glad/glad.h>
#include <iostream>

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetupOpenGL(HWND hWnd);
void CleanupOpenGL(HWND hWnd);
void Render();

// Global variables
HDC hDC;
HGLRC hRC;
GLuint shader;
GLuint vbo, vao;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASS wc = { 0 };
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "Win32_OpenGL";
    RegisterClass(&wc);

    // Create window
    HWND hWnd = CreateWindow(
        wc.lpszClassName,
        "OpenGL Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    // Setup OpenGL
    SetupOpenGL(hWnd);

    // Show window
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop
    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Render();
            SwapBuffers(hDC);
        }
    }

    // Cleanup OpenGL
    CleanupOpenGL(hWnd);

    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void SetupOpenGL(HWND hWnd) {
    // Get the device context (DC)
    hDC = GetDC(hWnd);

    // Set the pixel format for the DC
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int format = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, format, &pfd);

    // Create and enable the render context (RC)
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    if (!gladLoadGL())
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    // Setup basic OpenGL settings
    glViewport(0, 0, 800, 600);

    const char* vertexShaderSource = "#version 330 core\n"
        "layout(location=0) in vec3 aPos;\n"
        "void main(){\n"
        "gl_Position=vec4(aPos, 1.0);\n"
        "}\0";

    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 fragColor;\n"
        "void main(){\n"
        "fragColor = vec4(1.0,0.0,1.0,1.0);\n"
        "}\0";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);

    ::shader = glCreateProgram();
    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glLinkProgram(shader);
    
    glDeleteShader(vs);
    glDeleteShader(fs);

    float vertices[] = {
                        -0.5, -0.2, 0.0,
                         0.5, -0.2, 0.0,
                         0.0,  0.3, 0.0
    };

    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)NULL);
    glEnableVertexAttribArray(0);
    
}

void CleanupOpenGL(HWND hWnd) {
    // Disable RC and release DC
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader);
    glEnableVertexAttribArray(vao);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}
