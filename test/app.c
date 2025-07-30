#include "lucidacta/lucidacta.h"

#include "lucid_experimental.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance; (void)hPrevInstance; (void)pCmdLine; (void)nCmdShow;

	LucidInit();
	LucidStartPerformanceCounter();
	LucidCreateWindow("Lucidacta", 1280, 720);
	LucidCreateRenderer();
	LucidSetupImGui();
		
	LucidCreateTriangle();
	
	g_lucidContext->profile.startupElapsedTime = LucidEndPerformanceCounter();

	MSG msg;

	for(;;)
	{
		LucidUpdateDeltaTime();

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (LucidWindowShouldClose()) break;

		// IMGUI
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		igNewFrame();

		// FPSOVERLAY
		LucidFPSOverlay();

		igRender();

		LucidBeginFrame();

		LucidDrawMeshes();
		
		ImGui_ImplVulkan_RenderDrawData(igGetDrawData(), g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], NULL);
		LucidEndFrame();

		igUpdatePlatformWindows();
		igRenderPlatformWindowsDefault(NULL, NULL);
	}

	
	LucidGuiShutdown();
	LucidDestroyRenderer();
	LucidDestroyWindow();
	LucidShutdown();
	return 0;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;
	switch (uMsg)
	{
	case WM_DESTROY:
		LucidRequestWindowClose();
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}