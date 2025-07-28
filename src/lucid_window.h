#ifndef LUCID_WINDOW_H_
#define LUCID_WINDOW_H_

#include "lucid_result.h"
#include <Windows.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct LucidWindow_T
{
	const char* title;
	uint32_t width;
	uint32_t height;
	HWND handle;
	bool close;
} LucidWindow_T;
typedef struct LucidWindow_T* LucidWindow;

LucidResult LucidCreateWindow(const char* title, uint32_t width, uint32_t height);
void LucidDestroyWindow();

#endif // LUCID_WINDOW_H_