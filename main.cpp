#include <windows.h>
#include <glad/glad.h>

#include <iostream>
#include <array>

// VTK includes
#include "vtkWindows.h"
#include <ExternalVTKWidget.h>
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkExternalOpenGLRenderWindow.h>
#include <vtkLight.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkNamedColors.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkMetaImageReader.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>


// Function prototypes
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetupOpenGL(HWND hWnd);
void CleanupOpenGL(HWND hWnd);
void Render();
void vtkInitialize();

// Global variables
HDC hDC;
HGLRC hRC;
GLuint shader;
GLuint vbo, vao;

vtkNew<ExternalVTKWidget> externalVTKWidget;
static bool initialized = false;

static void MakeCurrentCallback(vtkObject* vtkNotUsed(caller), long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData)) {
    if (initialized) {
        wglMakeCurrent(hDC, hRC);
    }
}


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
                     -1.0, 1.0, -0.1,
                     1.0, 1.0, -0.1,
                     -1.0, -1.0, -0.1
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

void vtkInitialize() {
    vtkNew<vtkExternalOpenGLRenderWindow> renderWindow;
    externalVTKWidget->SetRenderWindow(renderWindow.GetPointer());

    vtkNew<vtkCallbackCommand> callback;
    callback->SetCallback(MakeCurrentCallback);
    renderWindow->AddObserver(vtkCommand::WindowMakeCurrentEvent, callback.GetPointer());

    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renderWindow);
    externalVTKWidget->GetRenderWindow()->SetInteractor(iren);

    /*vtkNew<vtkPolyDataMapper> mapper;
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper.GetPointer());*/
    vtkRenderer* renderer = externalVTKWidget->AddRenderer();
    //renderer->AddActor(actor.GetPointer());

    //vtkNew<vtkCubeSource> cs;
    //mapper->SetInputConnection(cs->GetOutputPort());
    ////actor->RotateX(45.0f);
    ////actor->RotateY(45.0f);
    //renderer->ResetCamera();

    vtkNew<vtkNamedColors> colors;

    std::array<unsigned char, 4> bkg{ {51, 77, 102, 255} };
    colors->SetColor("BkgColor", bkg.data());

    vtkNew<vtkMetaImageReader> reader;
    reader->SetFileName("data/FullHead/FullHead.mhd");

    vtkNew<vtkFixedPointVolumeRayCastMapper> volumeMapper;
    volumeMapper->SetInputConnection(reader->GetOutputPort());

    vtkNew<vtkColorTransferFunction> volumeColor;
    volumeColor->AddRGBPoint(0, 0.0, 0.0, 0.0);
    volumeColor->AddRGBPoint(500, 240.0 / 255.0, 184.0 / 255.0, 160.0 / 255.0);
    volumeColor->AddRGBPoint(1000, 240.0 / 255.0, 184.0 / 255.0, 160.0 / 255.0);
    volumeColor->AddRGBPoint(1150, 1.0, 1.0, 240.0 / 255.0); // Ivory

    vtkNew<vtkPiecewiseFunction> volumeScalarOpacity;
    volumeScalarOpacity->AddPoint(0, 0.00);
    volumeScalarOpacity->AddPoint(500, 0.15);
    volumeScalarOpacity->AddPoint(1000, 0.15);
    volumeScalarOpacity->AddPoint(1150, 0.85);

    vtkNew<vtkPiecewiseFunction> volumeGradientOpacity;
    volumeGradientOpacity->AddPoint(0, 0.0);
    volumeGradientOpacity->AddPoint(90, 0.5);
    volumeGradientOpacity->AddPoint(100, 1.0);

    vtkNew<vtkVolumeProperty> volumeProperty;
    volumeProperty->SetColor(volumeColor);
    volumeProperty->SetScalarOpacity(volumeScalarOpacity);
    volumeProperty->SetGradientOpacity(volumeGradientOpacity);
    volumeProperty->SetInterpolationTypeToLinear();
    volumeProperty->ShadeOn();
    volumeProperty->SetAmbient(0.4);
    volumeProperty->SetDiffuse(0.6);
    volumeProperty->SetSpecular(0.2);

    vtkNew<vtkVolume> volume;
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);

    std::cout << volume->GetBounds() << std::endl;
    std::cout << volume->GetBounds() << std::endl;

    renderer->AddViewProp(volume);

    vtkCamera* camera = renderer->GetActiveCamera();
    double* c = volume->GetCenter();
    camera->SetViewUp(0, 0, -1);
    camera->SetPosition(c[0], c[1] - 400, c[2]);
    camera->SetFocalPoint(c[0], c[1], c[2]);
    camera->Azimuth(30.0);
    camera->Elevation(30.0);

    renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

    initialized = true;
}

void Render() {
    if (!initialized) 
        vtkInitialize();

    // glClearColor(0.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(shader);
    glEnableVertexAttribArray(vao);

   glDrawArrays(GL_TRIANGLES, 0, 3);

    //externalVTKWidget->GetRenderWindow()->GetInteractor()->Start();

    externalVTKWidget->GetRenderWindow()->Render();

    //SwapBuffers(hDC);
}
