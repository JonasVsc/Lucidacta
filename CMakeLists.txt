cmake_minimum_required (VERSION 3.31)
project ("Lucidacta" C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Vulkan)
add_subdirectory(3rdparty/cimgui)

include_directories(include src)


add_library(lucidacta 
	STATIC	"src/lucid_context.c"
			"src/lucid_window.c"
			"src/lucid_renderer.c"
			"src/lucid_pipeline.c"
			"src/lucid_vk_utils.c"
			"src/lucid_utils.c"

			"src/lucid_gui.c"
			"src/lucid_experimental.c"
)

target_include_directories(lucidacta 
	PRIVATE	src
)

target_link_libraries(lucidacta 
	PUBLIC	Vulkan::Vulkan cimgui
)

add_executable(app 
	WIN32	"test/app.c"
)

target_link_libraries(app 
	PRIVATE	lucidacta
)


find_program(
    GLSLC_EXECUTABLE glslc
    REQUIRED
    DOC "Caminho para o compilador glslc"
)

add_custom_command(
    TARGET lucidacta POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:lucidacta>/shaders"
    COMMENT "Copiando diretório de shaders para o diretório do executável"
)

add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:cimgui>"
    "$<TARGET_FILE_DIR:app>"
    COMMENT "Copiando cimgui.dll para o diretório do executável"
)