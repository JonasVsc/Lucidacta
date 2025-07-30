#ifndef LUCID_GUI_H_
#define LUCID_GUI_H_

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_USE_WIN32
#define CIMGUI_USE_VULKAN


#include <cimgui.h>
#include <cimgui_impl.h>

void LucidSetupImGui();

void LucidGuiShutdown();

#endif // LUCID_GUI_H_