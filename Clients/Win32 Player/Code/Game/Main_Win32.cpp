#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "Engine/Math/Vector2.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Audio/Audio.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/Console.hpp"
#include "Game/TheGame.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/Memory/MemoryTracking.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/3D/ForwardRenderer.hpp"
#include <shellapi.h>
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "Engine/Core/Events/EventSystem.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Audio/SongManager.hpp"
#include <gl/GL.h>
#include "ThirdParty/OpenGL/wglext.h"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Audio/AudioMetadataUtils.hpp"
#include "ThirdParty/CEF/tests/cefsimple/simple_app.h"
#include "ThirdParty/CEF/include/cef_app.h"
#include "ThirdParty/CEF/include/internal/cef_types_wrappers.h"
#include "ThirdParty/CEF/include/cef_sandbox_win.h"

#pragma comment( lib, "ThirdParty/CEF/libcef_dll_wrapper" )

//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);

//-----------------------------------------------------------------------------------------------
const int OFFSET_FROM_WINDOWS_DESKTOP = 50;
int WINDOW_PHYSICAL_WIDTH = 1600;
int WINDOW_PHYSICAL_HEIGHT = 900;
const float VIEW_LEFT = 0.0;
const float VIEW_RIGHT = 1600.0;
const float VIEW_BOTTOM = 0.0;
const float VIEW_TOP = VIEW_RIGHT * static_cast<float>(WINDOW_PHYSICAL_HEIGHT) / static_cast<float>(WINDOW_PHYSICAL_WIDTH);
const Vector2 BOTTOM_LEFT = Vector2(VIEW_LEFT, VIEW_BOTTOM);
const Vector2 TOP_RIGHT = Vector2(VIEW_RIGHT, VIEW_TOP);
const unsigned int MAX_MESSAGE_SIZE = MAX_PATH + 1;

bool g_isQuitting = false;
bool g_isFullscreen = false;
bool g_uiHidden = false;
HWND g_hWnd = nullptr;
HDC g_displayDeviceContext = nullptr;
HGLRC g_openGLRenderingContext = nullptr;
const char* APP_NAME = "Turntable";

//-----------------------------------------------------------------------------------
void HandleFileDrop(WPARAM wParam)
{
    //Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/bb776408(v=vs.85).aspx
    HDROP fileDrop = (HDROP)wParam;
    UINT fileNumberToQuery = 0;  
    TCHAR tcharFilePath[MAX_PATH] = TEXT("");
    UINT numFilesInDrop = DragQueryFile(fileDrop, 0xFFFFFFFF, NULL, NULL); //Pass 0xFFFFFFFF and DragQueryFile will return total number of files dropped.

    DragQueryFile(fileDrop, fileNumberToQuery, tcharFilePath, MAX_PATH);
    std::wstring filePath(tcharFilePath);
    if (IsDirectory(filePath))
    {
        std::vector<std::wstring> playableFiles = GetSupportedAudioFiles(filePath);
        bool isPlaying = SongManager::instance->IsPlaying();
        for (std::wstring& file : playableFiles)
        {
            std::wstring fullFilePath = filePath + L"\\" + file;
            if (!isPlaying)
            {
                Console::instance->RunCommand(WStringf(L"play \"%s\"", fullFilePath.c_str()));
                isPlaying = true;
            }
            else
            {
                Console::instance->RunCommand(WStringf(L"addtoqueue \"%s\"", fullFilePath.c_str()));
            }
        }
    }
    else if (!SongManager::instance->IsPlaying())
    {
        Console::instance->RunCommand(WStringf(L"play \"%s\"", filePath.c_str()), true);
    }
    else
    {
        Console::instance->RunCommand(WStringf(L"addtoqueue \"%s\"", filePath.c_str()));
    }

    while (--numFilesInDrop > 0)
    {
        DragQueryFile(fileDrop, ++fileNumberToQuery, tcharFilePath, MAX_PATH);
        filePath = std::wstring(tcharFilePath);
        if (IsDirectory(filePath))
        {
            std::vector<std::wstring> playableFiles = GetSupportedAudioFiles(filePath);
            for (std::wstring& file : playableFiles)
            {
                std::wstring fullFilePath = filePath + L"\\" + file;
                Console::instance->RunCommand(WStringf(L"addtoqueue \"%s\"", fullFilePath.c_str()));
            }
        }
        else
        {
            Console::instance->RunCommand(WStringf(L"addtoqueue \"%s\"", filePath.c_str()));
        }
    }
    DragFinish(fileDrop);
}

//-----------------------------------------------------------------------------------
void PollForMessageJob(Job* job)
{
    //Try to create named pipe for Turntable
    HANDLE turntablePipe = (HANDLE) job->data;
    bool pipeConnected = false;
    OVERLAPPED overlapped;
    overlapped.hEvent = CreateEvent(NULL, true, false, L"TurntableEvent");
    overlapped.Internal = 0;
    overlapped.InternalHigh = 0;
    overlapped.Offset = 0;
    overlapped.OffsetHigh = 0;
    overlapped.Pointer = 0;

    while (!g_isQuitting)
    {
        //Wait for a user to connect and send its message
        if (GetLastError() != ERROR_PIPE_CONNECTED)
        {
            pipeConnected = ConnectNamedPipe(turntablePipe, &overlapped);
        }
        else
        {
            TCHAR* request = (TCHAR*)HeapAlloc(GetProcessHeap(), 0, MAX_MESSAGE_SIZE * sizeof(TCHAR));
            LPWSTR reply = L"Success";
            DWORD bytesRead = 0;
            DWORD bytesWritten = 0;

            ReadFile(
                turntablePipe,      //Handle to pipe
                request,        //Buffer to read from
                MAX_MESSAGE_SIZE * sizeof(TCHAR),       //Max size of buffer
                &bytesRead,     //Number of bytes read
                NULL);      //Not overlapped I/O

            WriteFile(
                turntablePipe,        // handle to pipe 
                reply,     // buffer to write from 
                sizeof(reply), // number of bytes to write 
                &bytesWritten,   // number of bytes written 
                NULL);        // not overlapped I/O

            //Request is assumed to be an absolute path to a song to play
            if (!SongManager::instance->IsPlaying())
            {
                Console::instance->RunCommand(WStringf(L"play \"%s\"", request));
            }
            else
            {
                Console::instance->RunCommand(WStringf(L"addtoqueue \"%s\"", request));
            }

            //We flush the file buffer here in case the pipe is not in a clean state
            FlushFileBuffers(turntablePipe);
            DisconnectNamedPipe(turntablePipe);
            pipeConnected = false;
        }
    }
}

//-----------------------------------------------------------------------------------
void DispatchPollForMessageJob(HANDLE pipe)
{
    JobSystem::instance->CreateAndDispatchJob(GENERIC_SLOW, &PollForMessageJob, pipe);
}

//-----------------------------------------------------------------------------------------------
HANDLE CreateTurntablePipe()
{
    return CreateNamedPipe(L"\\\\.\\pipe\\TURNTABLE\\",                             //Name
        PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED,  //Open mode (bidirectional, one instance created only, threaded)
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT | PIPE_ACCEPT_REMOTE_CLIENTS, //Pipe mode (Write and read as messages, nonblocking, connections accepted from other turntable instances)
        PIPE_UNLIMITED_INSTANCES,                   //Number of max instances
        MAX_MESSAGE_SIZE * sizeof(TCHAR),           //Output buffer size
        MAX_MESSAGE_SIZE * sizeof(TCHAR),           //Input buffer size
        0,                                          //Default timeout (50 ms)
        NULL);                                      //Default security attributes
}

//-----------------------------------------------------------------------------------------------
void SendArgsToOpenInstance()
{
    TCHAR readBuffer[MAX_MESSAGE_SIZE];
    DWORD cbRead;
    int numArgs;
    LPWSTR* argList;

    argList = CommandLineToArgvW(GetCommandLineW(), &numArgs);

    //If there was a file passed to this instance of Turntable, send it to the open one
    if (numArgs > 1 && argList != NULL)
    {
        for (int i = 1; i < numArgs; ++i)
        {
            //Send the file to open to the first Turntable instance
            LPWSTR message = argList[i];
            CallNamedPipe(
                L"\\\\.\\pipe\\TURNTABLE\\",                  // pipe name 
                message,              // message to server
                (lstrlen(message) + 1) * sizeof(TCHAR), // message length 
                readBuffer,              // buffer to receive reply
                MAX_MESSAGE_SIZE * sizeof(TCHAR),  // size of read buffer
                &cbRead,                // bytes read
                NMPWAIT_USE_DEFAULT_WAIT);                  // timeout
        }
    }

    LocalFree(argList);
}

//-----------------------------------------------------------------------------------------------
void EnsureCorrectWorkingDirectory()
{
    TCHAR buffer[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buffer);
    std::wstring currentPath = std::wstring(buffer) + L"\\Turntable.exe";

    if (!FileExists(currentPath))
    {
        //If Turntable isn't in our working directory, change to the calling directory
        int numArgs;
        LPWSTR* argList;
        argList = CommandLineToArgvW(GetCommandLineW(), &numArgs);
        std::wstring path = GetFileDirectory(argList[0]);
        SetCurrentDirectory(path.c_str());
        LocalFree(argList);
    }
}

//-----------------------------------------------------------------------------------
void HandleFileAssociation()
{
    int numArgs;
    LPWSTR* argList;

    argList = CommandLineToArgvW(GetCommandLineW(), &numArgs);

    //First arg is always the absolute path to Turntable, so we start at index 1
    if (numArgs > 1 && argList != NULL)
    {
        for (int i = 0; i < numArgs - 1; ++i)
        {
            if (!SongManager::instance->IsPlaying())
            {
                Console::instance->RunCommand(WStringf(L"play \"%s\"", argList[i+1]), true);
            }
            else
            {
                Console::instance->RunCommand(WStringf(L"addtoqueue \"%s\"", argList[i+1]));
            }
        }
    }

    LocalFree(argList);
}

//-----------------------------------------------------------------------------------
void HandleMouseWheel(WPARAM wParam)
{
    short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    InputSystem::instance->SetMouseWheelStatus(zDelta);
}

//-----------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
    unsigned char asKey = (unsigned char)wParam;
    switch (wmMessageCode)
    {
    case WM_CLOSE:
    case WM_DESTROY:
    case WM_QUIT:
        g_isQuitting = true;
        return 0;

    case WM_CHAR:
        InputSystem::instance->SetLastPressedChar(asKey);
        g_uiHidden = false;
        break;

    case WM_KEYDOWN:
        InputSystem::instance->SetKeyDownStatus(asKey, true);
        g_uiHidden = false;
        if (asKey == VK_ESCAPE)
        {
            g_isQuitting = true;
            return 0;
        }
        break;

    case WM_KEYUP:
        InputSystem::instance->SetKeyDownStatus(asKey, false);
        break;

    case WM_LBUTTONDOWN:
        InputSystem::instance->SetMouseDownStatus(InputSystem::MouseButton::LEFT_MOUSE_BUTTON, true);
        break;

    case WM_RBUTTONDOWN:
        InputSystem::instance->SetMouseDownStatus(InputSystem::MouseButton::RIGHT_MOUSE_BUTTON, true);
        break;

    case WM_MBUTTONDOWN:
        InputSystem::instance->SetMouseDownStatus(InputSystem::MouseButton::MIDDLE_MOUSE_BUTTON, true);
        break;

    case WM_LBUTTONUP:
        InputSystem::instance->SetMouseDownStatus(InputSystem::MouseButton::LEFT_MOUSE_BUTTON, false);
        break;

    case WM_RBUTTONUP:
        InputSystem::instance->SetMouseDownStatus(InputSystem::MouseButton::RIGHT_MOUSE_BUTTON, false);
        break;

    case WM_MBUTTONUP:
        InputSystem::instance->SetMouseDownStatus(InputSystem::MouseButton::MIDDLE_MOUSE_BUTTON, false);
        break;

    case WM_MOUSEWHEEL:
        HandleMouseWheel(wParam);
        break;

    case WM_DROPFILES:
        HandleFileDrop(wParam);
        break;
    case WM_SETCURSOR:
        break;
    }

    return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}

//-----------------------------------------------------------------------------------------------
void CreateOpenGLWindow(HINSTANCE applicationInstanceHandle)
{
    // Define a window class
    WNDCLASSEX windowClassDescription;
    memset(&windowClassDescription, 0, sizeof(windowClassDescription));
    windowClassDescription.cbSize = sizeof(windowClassDescription);
    windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
    windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Assign a win32 message-handling function
    windowClassDescription.hInstance = GetModuleHandle(NULL);
    windowClassDescription.hIcon = NULL;
    windowClassDescription.hCursor = NULL;
    windowClassDescription.lpszClassName = TEXT("Simple Window Class");
    RegisterClassEx(&windowClassDescription);

    RECT desktopRect;
    HWND desktopWindowHandle = GetDesktopWindow();
    GetClientRect(desktopWindowHandle, &desktopRect);

    Vector2 desktopSize = Vector2(desktopRect.right - desktopRect.left, desktopRect.bottom - desktopRect.top);
    float maxWindowPercentage = 0.85f;
    Vector2 maxWindowSize = desktopSize * maxWindowPercentage;
    float maxWindowAspect = maxWindowSize.x / maxWindowSize.y;
    float desiredAspect = (float)WINDOW_PHYSICAL_WIDTH / (float)WINDOW_PHYSICAL_HEIGHT;

    Vector2 windowSize = maxWindowSize;
    if (desiredAspect > maxWindowAspect) //Too wide
    {
        windowSize.y = maxWindowSize.x / desiredAspect;
    }
    else
    {
        windowSize.x = maxWindowSize.y * desiredAspect;
    }

    Vector2 marginDimensions = desktopSize - windowSize;

    float top = marginDimensions.y / 2.0f;
    float left = marginDimensions.x / 2.0f;
    float bottom = top + windowSize.y;
    float right = left + windowSize.x;

    RECT windowRect = { (int)left, (int)top, (int)right, (int)bottom };
    DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
    DWORD windowStyleExFlags = WS_EX_APPWINDOW;
    if (g_isFullscreen)
    {
        windowStyleFlags = WS_POPUP;
        windowStyleExFlags = WS_EX_APPWINDOW;
        windowRect = desktopRect;
    }

    WINDOW_PHYSICAL_WIDTH = windowRect.right - windowRect.left;
    WINDOW_PHYSICAL_HEIGHT = windowRect.bottom - windowRect.top;

    AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags); //Compensates for the windows frame, allowing the above rectangle to be just the client space.

    WCHAR windowTitle[1024];
    MultiByteToWideChar(GetACP(), 0, APP_NAME, -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
    g_hWnd = CreateWindowEx(
        windowStyleExFlags,
        windowClassDescription.lpszClassName,
        windowTitle,
        windowStyleFlags,
        windowRect.left,
        windowRect.top,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        applicationInstanceHandle,
        NULL);

    ShowWindow(g_hWnd, SW_SHOW);
    SetForegroundWindow(g_hWnd);
    SetFocus(g_hWnd);

    g_displayDeviceContext = GetDC(g_hWnd);

    HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
    SetCursor(cursor);

    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    memset(&pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor));
    pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
    pixelFormatDescriptor.nVersion = 1;
    pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
    pixelFormatDescriptor.cColorBits = 24;
    pixelFormatDescriptor.cDepthBits = 24;
    pixelFormatDescriptor.cAccumBits = 0;
    pixelFormatDescriptor.cStencilBits = 8;

    int pixelFormatCode = ChoosePixelFormat(g_displayDeviceContext, &pixelFormatDescriptor);
    SetPixelFormat(g_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor);
    g_openGLRenderingContext = wglCreateContext(g_displayDeviceContext);
    wglMakeCurrent(g_displayDeviceContext, g_openGLRenderingContext);


    PFNWGLCREATECONTEXTATTRIBSARBPROC createContextAttribsARBpointer = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_openGLRenderingContext);

    static const int attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,

        NULL
    };
    g_openGLRenderingContext = createContextAttribsARBpointer(g_displayDeviceContext, NULL, attributes);
    wglMakeCurrent(g_displayDeviceContext, g_openGLRenderingContext);

    DragAcceptFiles(g_hWnd, TRUE);
}


//-----------------------------------------------------------------------------------------------
void RunMessagePump()
{
    MSG queuedMessage;
    for (;;)
    {
        const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
        if (!wasMessagePresent)
        {
            break;
        }

        TranslateMessage(&queuedMessage);
        DispatchMessage(&queuedMessage);
    }
}

//-----------------------------------------------------------------------------------------------
void Update()
{
    static double s_timeLastFrameStarted = GetCurrentTimeSeconds();
    double timeNow = GetCurrentTimeSeconds();
    float deltaSeconds = (float)( timeNow - s_timeLastFrameStarted );
    s_timeLastFrameStarted = timeNow;
    UpdateFrameRate(deltaSeconds);

    InputSystem::instance->Update(deltaSeconds);
    AudioSystem::instance->Update(deltaSeconds);
    Console::instance->Update(deltaSeconds);
    TheGame::instance->Update(deltaSeconds);
    UISystem::instance->Update(deltaSeconds);
}

//-----------------------------------------------------------------------------------------------
void Render()
{
    Renderer::instance->ClearScreen(0.0f, 0.8f, 0.0f); //Clear screen for FBO
    TheGame::instance->m_fbo->Bind();
    Renderer::instance->ClearScreen(0.3f, 0.3f, 0.3f);
    TheGame::instance->RenderPostProcess();
    Renderer::instance->FrameBufferCopyToBack(TheGame::instance->m_fbo, TheGame::instance->m_fbo->m_pixelWidth, TheGame::instance->m_fbo->m_pixelHeight);
    TheGame::instance->m_fbo->Unbind();

    if (g_uiHidden == false)
    {
        Renderer::instance->m_defaultMaterial->m_renderState.depthTestingMode = RenderState::DepthTestingMode::ON;
        TheGame::instance->Render();
        Renderer::instance->m_defaultMaterial->m_renderState.depthTestingMode = RenderState::DepthTestingMode::OFF;
        UISystem::instance->Render();
        Console::instance->Render();
    }

    SwapBuffers(g_displayDeviceContext);
}

//-----------------------------------------------------------------------------------------------
void RunFrame()
{
    InputSystem::instance->AdvanceFrame();
    RunMessagePump();
    Update();
    Render();
}

//-----------------------------------------------------------------------------------------------
void Initialize(HINSTANCE applicationInstanceHandle)
{
    SetProcessDPIAware();
    CreateOpenGLWindow(applicationInstanceHandle);
    Renderer::instance = new Renderer(Vector2Int(WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT));
    ForwardRenderer::instance = new ForwardRenderer();
    AudioSystem::instance = new AudioSystem();
    InputSystem::instance = new InputSystem(g_hWnd, 0, WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT);
    Console::instance = new Console();
    UISystem::instance = new UISystem();
    JobSystem::instance = new JobSystem(0);
    JobSystem::instance->Initialize();
    TheGame::instance = new TheGame();
    HandleFileAssociation();
}

//-----------------------------------------------------------------------------------
void EngineCleanup()
{
    Texture::CleanUpTextureRegistry();
    BitmapFont::CleanUpBitmapFontRegistry();
    EventSystem::CleanUpEventRegistry();
}

//-----------------------------------------------------------------------------------------------
void Shutdown()
{
    delete TheGame::instance;
    TheGame::instance = nullptr;                                            
    JobSystem::instance->Shutdown();
    delete JobSystem::instance;
    JobSystem::instance = nullptr;
    delete UISystem::instance;
    UISystem::instance = nullptr;
    delete Console::instance;
    Console::instance = nullptr;
    delete InputSystem::instance;
    InputSystem::instance = nullptr;
    delete AudioSystem::instance;
    AudioSystem::instance = nullptr;
    delete ForwardRenderer::instance;
    ForwardRenderer::instance = nullptr;
    delete Renderer::instance;
    Renderer::instance = nullptr;
    EngineCleanup();
}

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE hInstance, PSTR commandLineString, int)
{
    CefMainArgs main_args(hInstance);
    UNUSED(commandLineString); 
    CefRefPtr<SimpleApp> app(new SimpleApp());
// 
//     // Specify CEF global settings here.
//     CefSettings settings;
//     void* sandbox_info = NULL;
//     settings.no_sandbox = true;
// 
//     // Initialize CEF.
//     CefInitialize(main_args, settings, app.get(), sandbox_info);
// 
//     // Run the CEF message loop. This will block until CefQuitMessageLoop() is
//     // called.
//     CefRunMessageLoop();
// 
//     // Shut down CEF.
//     CefShutdown();
    

    HANDLE turntablePipe = CreateTurntablePipe();

    if (GetLastError() == ERROR_ACCESS_DENIED || turntablePipe == INVALID_HANDLE_VALUE)
    {
        //The pipe exists, so send a message to the open instance and close this one
        SendArgsToOpenInstance();
    }
    else
    {
        EnsureCorrectWorkingDirectory();
        MemoryAnalyticsStartup();
        Initialize(applicationInstanceHandle);
        DispatchPollForMessageJob(turntablePipe);

        while (!g_isQuitting)
        {
            RunFrame();
        }

        CloseHandle(turntablePipe);
        Shutdown();
        MemoryAnalyticsShutdown();
    }

    return 0;
}