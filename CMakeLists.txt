cmake_minimum_required (VERSION 3.31)
project ("Lucidacta")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Vulkan)

include_directories(include src)

add_library(lucidacta STATIC
	"src/lucid_context.c"
	"src/lucid_window.c"
	"src/lucid_renderer.c"
)

target_include_directories(lucidacta 
	PRIVATE src)

target_link_libraries(lucidacta 
	PUBLIC Vulkan::Vulkan)

add_executable(app WIN32 "test/app.c")
target_link_libraries(app PRIVATE lucidacta)