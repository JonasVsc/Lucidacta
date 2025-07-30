#include "lucid_context.h"
#include <stdlib.h>

LucidContext g_lucidContext = NULL;

LucidResult LucidInit()
{
	g_lucidContext = calloc(1, sizeof(LucidContext_T));
	if (g_lucidContext == NULL)
		return LUCID_ERROR;

	LucidInitProfileTools();

	return LUCID_SUCCESS;
}

inline void LucidInitProfileTools()
{
	QueryPerformanceFrequency(&g_lucidContext->profile.frequency);
}

void LucidShutdown()
{
	if (g_lucidContext == NULL)
		return;
	free(g_lucidContext);
	g_lucidContext = NULL;
}

