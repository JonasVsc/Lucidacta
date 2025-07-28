#include "lucidacta/lucidacta.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance; (void)hPrevInstance; (void)pCmdLine; (void)nCmdShow;

	LucidInit();
	LucidCreateWindow("Lucidacta", 640, 480);
	LucidCreateRenderer();

	MSG msg;
	while (!LucidWindowShouldClose())
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (LucidWindowShouldClose()) break;

		LucidBeginFrame();

		LucidEndFrame();
	}

	LucidDestroyRenderer();
	LucidDestroyWindow();
	LucidShutdown();
	return 0;
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		LucidRequestWindowClose();
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}