#include "lucid_context.h"
#include <stdlib.h>

LucidContext g_lucidContext = NULL;

LucidResult LucidInit()
{
	g_lucidContext = calloc(1, sizeof(LucidContext_T));
	if (g_lucidContext == NULL)
		return LUCID_ERROR;

	QueryPerformanceFrequency(&g_lucidContext->frequency);
	QueryPerformanceCounter(&g_lucidContext->lastFrame);

	return LUCID_SUCCESS;
}

void LucidShutdown()
{
	if (g_lucidContext == NULL)
		return;
	free(g_lucidContext);
	g_lucidContext = NULL;
}

