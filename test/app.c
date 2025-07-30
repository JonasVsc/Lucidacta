#include "lucidacta/lucidacta.h"
#include "gui/lucid_gui.h"

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
	
	double microSeconds = LucidEndPerformanceCounter();


	float timeAccumulator = 0.0f;
	float displayedFPS = 0.0f;

	ImGuiIO* io = igGetIO_Nil();
	MSG msg;

	for(;;)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (LucidWindowShouldClose()) break;

		timeAccumulator += io->DeltaTime;
		if (timeAccumulator >= 0.5f)
		{
			displayedFPS = io->Framerate;
			timeAccumulator = 0.0f;
		}

		// IMGUI
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		igNewFrame();
		
		// FPSOVERLAY
		{
			ImVec2 vec = { 0, 0 };
			ImVec2 pivot = { 0, 0 };
			igSetNextWindowPos(vec, ImGuiCond_Always, pivot);
			igBegin("FPSOverlay", NULL,
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoBackground
			);

			igText("FPS %.1f | Frame Time %.3f ms", displayedFPS, 1000.0f / displayedFPS);

			igEnd();
		}

		// START TIME
		{
			ImVec2 vec = { g_lucidContext->window.width - 15.0f, 0.0f };
			ImVec2 pivot = { 1.0f, 0.0f };
			igSetNextWindowPos(vec, ImGuiCond_Always, pivot);
			igBegin("StartTime", NULL,
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoBackground
			);

			igText("Start Time %.1f s", microSeconds / 100000.0f);

			igEnd();
		}
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