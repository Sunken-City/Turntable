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
#include "Game/TheApp.hpp"
#include "Game/TheGame.hpp"

//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);

//-----------------------------------------------------------------------------------------------
const int OFFSET_FROM_WINDOWS_DESKTOP = 50;
const int WINDOW_PHYSICAL_WIDTH = 1600;
const int WINDOW_PHYSICAL_HEIGHT = 900;
const float VIEW_LEFT = 0.0;
const float VIEW_RIGHT = 1600.0;
const float VIEW_BOTTOM = 0.0;
const float VIEW_TOP = VIEW_RIGHT * static_cast<float>(WINDOW_PHYSICAL_HEIGHT) / static_cast<float>(WINDOW_PHYSICAL_WIDTH);
const Vector2 BOTTOM_LEFT = Vector2(VIEW_LEFT, VIEW_BOTTOM);
const Vector2 TOP_RIGHT = Vector2(VIEW_RIGHT, VIEW_TOP);

bool g_isQuitting = false;
HWND g_hWnd = nullptr;
HDC g_displayDeviceContext = nullptr;
HGLRC g_openGLRenderingContext = nullptr;
const char* APP_NAME = "T W A H  B O Y S";


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
		break;

	case WM_KEYDOWN:
		InputSystem::instance->SetKeyDownStatus(asKey, true);
		/*if (asKey == VK_ESCAPE)
		{
			g_isQuitting = true;
			return 0;
		}*/
		break;

	case WM_KEYUP:
		InputSystem::instance->SetKeyDownStatus(asKey, false);
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

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);

	RECT windowRect = { OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_WIDTH, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_HEIGHT };
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

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

	DebugRenderer::instance->Update(deltaSeconds);
	AudioSystem::instance->Update(deltaSeconds);
	InputSystem::instance->Update(deltaSeconds);
	Console::instance->Update(deltaSeconds);
	TheGame::instance->Update(deltaSeconds);
}

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
//-----------------------------------------------------------------------------------------------
void Render()
{
	Renderer::instance->ClearScreen(0.0f, 0.0f, 0.0f); //Clear screen for FBO
	Renderer::instance->BindFramebuffer(TheGame::instance->m_fbo);
	Renderer::instance->ClearScreen(0.3f, 0.3f, 0.3f);

	TheGame::instance->Render();
	
	Renderer::instance->BindFramebuffer(nullptr);
	TheGame::instance->RenderPostProcess();

	SwapBuffers(g_displayDeviceContext);
}


//-----------------------------------------------------------------------------------------------
void RunFrame()
{
	InputSystem::instance->AdvanceFrameNumber();
	RunMessagePump();
	Update();
	Render();
}


//-----------------------------------------------------------------------------------------------
void Initialize(HINSTANCE applicationInstanceHandle)
{
	SetProcessDPIAware();
	CreateOpenGLWindow(applicationInstanceHandle);
	Renderer::instance = new Renderer();
	DebugRenderer::instance = new DebugRenderer();
	AudioSystem::instance = new AudioSystem();
	InputSystem::instance = new InputSystem(g_hWnd);
	Console::instance = new Console();
	TheApp::instance = new TheApp(VIEW_RIGHT, VIEW_TOP);
	TheGame::instance = new TheGame();
}


//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	delete TheGame::instance;
	TheGame::instance = nullptr;
	delete TheApp::instance;
	TheApp::instance = nullptr;
	delete Console::instance;
	Console::instance = nullptr;
	delete InputSystem::instance;
	InputSystem::instance = nullptr;
	delete AudioSystem::instance;
	AudioSystem::instance = nullptr;
	delete DebugRenderer::instance;
	DebugRenderer::instance = nullptr;
	delete Renderer::instance;
	Renderer::instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int)
{
	UNUSED(commandLineString);
	Initialize(applicationInstanceHandle);

	while (!g_isQuitting)
	{
		RunFrame();
	}

	Shutdown();
	return 0;
}