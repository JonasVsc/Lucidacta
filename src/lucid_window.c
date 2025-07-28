#include "lucid_window.h"
#include "lucid_context.h"

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LucidResult LucidCreateWindow(const char *title, uint32_t width, uint32_t height)
{
    if (g_lucidContext == NULL)
        return LUCID_ERROR;

    LucidWindow window = &g_lucidContext->window;

    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
    const char CLASS_NAME[] = "Window Class";
    WNDCLASS windowClass = {0};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = CLASS_NAME;
    RegisterClass(&windowClass);


    window->title = title;
    window->width = width;
    window->height = height;
    window->close = false;
    window->handle = CreateWindow(CLASS_NAME, window->title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                  window->width, window->height, NULL, NULL, hInstance, NULL);


    ShowWindow(window->handle, SW_SHOWNORMAL);

    return LUCID_SUCCESS;
}

void LucidDestroyWindow()
{
    DestroyWindow(g_lucidContext->window.handle);
}