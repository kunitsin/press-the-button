#include <Windows.h>
#include <tchar.h>
#include <GL/gl.h>

#ifdef __cplusplus
#error Compile it as C.
#endif

TCHAR *className  = _T("ptb");
TCHAR *windowName = _T("Press The Button");
int winX = 300, winY = 300;
int winWidth = 300, winHeight = 300;

HDC hDC;
HGLRC hGLRC;
HPALETTE hPalette;

BOOL buttonPressed;

void
init()
{
	/* set viewing projection */
	glMatrixMode(GL_PROJECTION);
	glFrustum(-0.5F, 0.5F, -0.5F, 0.5F, 1.0F, 3.0F);

	/* position viewer */
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(0.0F, 0.0F, -2.0F);

	/* position object */
	glRotatef(30.0F, 1.0F, 0.0F, 0.0F);
	glRotatef(30.0F, 0.0F, 1.0F, 0.0F);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
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
	if (pixelFormat == 0) {
		MessageBox(WindowFromDC(hDC), "ChoosePixelFormat failed.", "Error",
			MB_ICONERROR | MB_OK);
		exit(1);
	}

	if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) {
		MessageBox(WindowFromDC(hDC), "SetPixelFormat failed.", "Error",
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
		int i;

		for (i = 0; i < paletteSize; ++i)
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

	if (hPalette) {
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
	switch (message) {
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
			if (hGLRC) {
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(hGLRC);
			}
			if (hPalette) {
				DeleteObject(hPalette);
			}
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
			if (hGLRC && hPalette && (HWND)wParam != hWnd) {
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
			if (hGLRC && hPalette) {
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
	MSG msg;

	/* register window class */
	const WNDCLASS wndClass = {
		.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc   = WndProc,
		.hInstance     = hCurrentInst,
		.hIcon         = LoadIcon(NULL, IDI_APPLICATION),
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = GetStockObject(BLACK_BRUSH),
		.lpszClassName = className
	};
	RegisterClass(&wndClass);

	/* create window */
	hWnd = CreateWindow(
		className, windowName,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		winX, winY, winWidth, winHeight,
		NULL, NULL, hCurrentInst, NULL);

	/* display window */
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	/* process messages */
	while (GetMessage(&msg, NULL, 0, 0) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
