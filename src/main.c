// Some code is taken from http://www.cs.rit.edu/~ncs/Courses/570/UserGuide/OpenGLonWin-11.html.

#include <Windows.h>
#include <tchar.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <inttypes.h>

#include "freetype-2.5.5/include/ft2build.h"
#include FT_FREETYPE_H

#ifdef __cplusplus
#error Compile it as C.
#endif

#define MAX_FPS 120

TCHAR *className  = _T("PressTheButton");
TCHAR *windowName = _T("Press The Button");
int winWidth = 300, winHeight = 300;

HDC hDC;
HGLRC hGLRC;
HPALETTE hPalette;
BOOL buttonPressed;
FT_Library library;

void
init()
{
	/* position viewer */
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(0.0F, 0.0F, -2.0F);

	/* position object */
	glRotatef(30.0F, 1.0F, 0.0F, 0.0F);
	glRotatef(30.0F, 0.0F, 1.0F, 0.0F);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	if (FT_Init_FreeType(&library))
		exit(-1);

	FT_Face face;
	if (FT_New_Face(library, "Raleway-ExtraLight.ttf", 0, &face))
		exit(-1);
}

void
redraw()
{
	glClearColor(0.0F, 0.0F, buttonPressed ? 1.0F : 0.0F, 1.0F);
	/* clear color and depth buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* draw six faces of a cube */
	glBegin(GL_QUADS);
	glNormal3f(0.0F, 0.0F, 1.0F);
	glVertex3f(0.5F, 0.5F, 0.5F); glVertex3f(-0.5F, 0.5F, 0.5F);
	glVertex3f(-0.5F, -0.5F, 0.5F); glVertex3f(0.5F, -0.5F, 0.5F);

	glNormal3f(0.0F, 0.0F, -1.0F);
	glVertex3f(-0.5F, -0.5F, -0.5F); glVertex3f(-0.5F, 0.5F, -0.5F);
	glVertex3f(0.5F, 0.5F, -0.5F); glVertex3f(0.5F, -0.5F, -0.5F);

	glNormal3f(0.0F, 1.0F, 0.0F);
	glVertex3f(0.5F, 0.5F, 0.5F); glVertex3f(0.5F, 0.5F, -0.5F);
	glVertex3f(-0.5F, 0.5F, -0.5F); glVertex3f(-0.5F, 0.5F, 0.5F);

	glNormal3f(0.0F, -1.0F, 0.0F);
	glVertex3f(-0.5F, -0.5F, -0.5F); glVertex3f(0.5F, -0.5F, -0.5F);
	glVertex3f(0.5F, -0.5F, 0.5F); glVertex3f(-0.5F, -0.5F, 0.5F);

	glNormal3f(1.0F, 0.0F, 0.0F);
	glVertex3f(0.5F, 0.5F, 0.5F); glVertex3f(0.5F, -0.5F, 0.5F);
	glVertex3f(0.5F, -0.5F, -0.5F); glVertex3f(0.5F, 0.5F, -0.5F);

	glNormal3f(-1.0F, 0.0F, 0.0F);
	glVertex3f(-0.5F, -0.5F, -0.5F); glVertex3f(-0.5F, -0.5F, 0.5F);
	glVertex3f(-0.5F, 0.5F, 0.5F); glVertex3f(-0.5F, 0.5F, -0.5F);
	glEnd();

	SwapBuffers(hDC);
}

void
resize()
{
	/* set viewport to cover the window */
	glViewport(0, 0, winWidth, winHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, (GLdouble)winWidth / winHeight, 0.1, 7.0);
	glMatrixMode(GL_MODELVIEW);
}

void
setupPixelFormat(
	HDC hDC)
{
	const PIXELFORMATDESCRIPTOR pfd = {
		.nSize      = sizeof(PIXELFORMATDESCRIPTOR),
		.nVersion   = 1,
		.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER,
		.iPixelType = PFD_TYPE_RGBA,
		.cColorBits = 16,
		.cDepthBits = 16,
		.iLayerType = PFD_MAIN_PLANE
	};

	const int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (pixelFormat == 0)
	{
		MessageBox(
			WindowFromDC(hDC),
			"ChoosePixelFormat failed.",
			"Error",
			MB_ICONERROR | MB_OK);
		exit(1);
	}

	if (!SetPixelFormat(hDC, pixelFormat, &pfd))
	{
		MessageBox(
			WindowFromDC(hDC),
			"SetPixelFormat failed.",
			"Error",
			MB_ICONERROR | MB_OK);
		exit(1);
	}
}

void
setupPalette(
	HDC hDC)
{
	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(hDC, GetPixelFormat(hDC), sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	if (!(pfd.dwFlags & PFD_NEED_PALETTE))
		return;

	const int paletteSize = 1 << pfd.cColorBits;

	LOGPALETTE* pPal = malloc(sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
	pPal->palVersion = 0x300;
	pPal->palNumEntries = paletteSize;

	/* build a simple RGB color palette */
	{
		const int redMask   = (1 << pfd.cRedBits  ) - 1;
		const int greenMask = (1 << pfd.cGreenBits) - 1;
		const int blueMask  = (1 << pfd.cBlueBits ) - 1;

		for (int i = 0; i < paletteSize; ++i)
		{
			pPal->palPalEntry[i] = (PALETTEENTRY){
				(((i >> pfd.cRedShift  ) & redMask  ) * 255) / redMask,
				(((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask,
				(((i >> pfd.cBlueShift ) & blueMask ) * 255) / blueMask,
				0
			};
		}
	}

	hPalette = CreatePalette(pPal);
	free(pPal);

	if (hPalette)
	{
		SelectPalette(hDC, hPalette, FALSE);
		RealizePalette(hDC);
	}
}

LRESULT APIENTRY
WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			/* initialize OpenGL rendering */
			hDC = GetDC(hWnd);
			setupPixelFormat(hDC);
			setupPalette(hDC);
			hGLRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hGLRC);
			init();
			return 0;
		}
		case WM_DESTROY:
		{
			/* finish OpenGL rendering */
			if (hGLRC)
			{
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(hGLRC);
			}
			if (hPalette)
				DeleteObject(hPalette);

			ReleaseDC(hWnd, hDC);
			PostQuitMessage(0);
			return 0;
		}
		case WM_SIZE:
		{
			/* track window size changes */
			if (hGLRC)
			{
				winWidth = (int)LOWORD(lParam);
				winHeight = (int)HIWORD(lParam);
				resize();
				return 0;
			}
			break;
		}
		case WM_PALETTECHANGED:
		{
			/* realize palette if this is *not* the current window */
			if (hGLRC && hPalette && (HWND)wParam != hWnd)
			{
				UnrealizeObject(hPalette);
				SelectPalette(hDC, hPalette, FALSE);
				RealizePalette(hDC);
				redraw();
				break;
			}
			break;
		}
		case WM_QUERYNEWPALETTE:
		{
			/* realize palette if this is the current window */
			if (hGLRC && hPalette)
			{
				UnrealizeObject(hPalette);
				SelectPalette(hDC, hPalette, FALSE);
				RealizePalette(hDC);
				redraw();
				return TRUE;
			}
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			if (hGLRC)
				redraw();
			EndPaint(hWnd, &ps);
			return 0;
		}
		case WM_CHAR:
		{
			/* handle keyboard input */
			switch ((int)wParam)
			{
				case VK_ESCAPE:
				{
					DestroyWindow(hWnd);
					return 0;
				}
			}
			break;
		}
		case WM_KEYDOWN:
		{
			if (wParam == 0x57)
				buttonPressed = TRUE;
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;
		}
		case WM_KEYUP:
		{
			if (wParam == 0x57)
				buttonPressed = FALSE;
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY
WinMain(
	HINSTANCE hCurrentInst,
	HINSTANCE hPreviousInst,
	LPSTR lpszCmdLine,
	int nCmdShow)
{
	HWND hWnd;

	/* register window class */
	const WNDCLASS wndClass = {
		.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc   = WndProc,
		.hInstance     = hCurrentInst,
		.hIcon         = LoadIcon(NULL, IDI_APPLICATION),
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = NULL,
		.lpszClassName = className
	};
	if (!RegisterClass(&wndClass))
	{
		MessageBox(
			NULL,
			"RegisterClass failed.",
			"Error",
			MB_ICONERROR | MB_OK);
		exit(1);
	}

	winWidth  = GetSystemMetrics(SM_CXSCREEN);
	winHeight = GetSystemMetrics(SM_CYSCREEN);

	hWnd = CreateWindowEx(
		WS_EX_WINDOWEDGE | WS_EX_APPWINDOW,
		className,
		windowName,
		WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0, winWidth, winHeight,
		NULL, NULL, hCurrentInst, NULL);

	if (!hWnd)
	{
		MessageBox(
			NULL,
			"CreateWindow failed.",
			"Error",
			MB_ICONERROR | MB_OK);
		exit(1);
	}

	/* display window */
	ShowWindow(hWnd, nCmdShow);

	/* game loop */
	while (1)
	{
		/* query time */
		LARGE_INTEGER iterStart;
		QueryPerformanceCounter(&iterStart);

		/* process Windows messages */
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return msg.wParam;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		/* wait for MAX_FPS threshold */
		const int64_t sleepTill = iterStart.QuadPart + (int64_t)(1. / MAX_FPS * 1000000);
		LARGE_INTEGER iterEnd;
		QueryPerformanceCounter(&iterEnd);
		
		if (sleepTill > iterEnd.QuadPart)
			Sleep((sleepTill - iterEnd.QuadPart) / 1000);
	}
}
