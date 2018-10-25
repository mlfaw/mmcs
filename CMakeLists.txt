
cmake_minimum_required(VERSION 3.0)
project(mmcs_superbuild LANGUAGES C CXX)

# These are inserted into various files through configure_file
if (NOT DEFINED MMCS_YEAR)
	string(TIMESTAMP MMCS_YEAR  "%Y" UTC)
	string(TIMESTAMP MMCS_MONTH "%m" UTC)
	string(TIMESTAMP MMCS_DAY   "%d" UTC)
	string(TIMESTAMP MMCS_HOUR  "%H" UTC)
endif()
set(MMCS_DESCRIPTION "MMCS - media viewer and tagger")
set(MMCS_NAME        "MMCS - My Media Categories Suite")
set(MMCS_EXE         "mmcs.exe")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CreateCResources)
include(MSVCParallelBuild)
#include(MSVCStaticLibs)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/_generated")
set(GEN_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/_generated/generated")

add_definitions(
	-DMMCS_YEAR=${MMCS_YEAR}
	-DMMCS_MONTH=${MMCS_MONTH}
	-DMMCS_DAY=${MMCS_DAY}
	-DMMCS_HOUR=${MMCS_HOUR}
	-DMMCS_VERSION_DOTS="${MMCS_YEAR}.${MMCS_MONTH}.${MMCS_DAY}.${MMCS_HOUR}"
	-DMMCS_VERSION_UNDERSCORES="${MMCS_YEAR}_${MMCS_MONTH}_${MMCS_DAY}_${MMCS_HOUR}"
)

set(mmcs_libs)
file(GLOB mmcs_sources "src/mmcs_*")
list(APPEND mmcs_sources
	#"${GEN_FOLDER}/Changelog.c"
	#"${GEN_FOLDER}/Changelog.h"
	"${CMAKE_SOURCE_DIR}/thirdparty/stb_image.h"
	#"src/small_zip.c"
	#"src/small_zip.h"
)

if (WIN32)
	add_definitions(
		-DMMCS_MSW=1
		-DMMCS_WIN32=1
		-D_UNICODE
		-DUNICODE
		-D_CRT_SECURE_NO_WARNINGS
		-DWINVER=0x0601
		-D_WIN32_WINNT=0x0601
	)
	list(APPEND mmcs_libs d3d11 d2d1 dxgi dwrite)
	file(GLOB win_sources "src/msw_*" "src/win32_*")
	list(APPEND mmcs_sources
		${win_sources}
		"${GEN_FOLDER}/win32_resource.h"
		"${GEN_FOLDER}/win32_resource.rc"
		"${GEN_FOLDER}/win32.manifest"
	)
	add_definitions()
	configure_file(src/gen_win32_resource.in.h  "${GEN_FOLDER}/win32_resource.h")
	configure_file(src/gen_win32_resource.in.rc "${GEN_FOLDER}/win32_resource.rc")
	configure_file(src/gen_win32.in.manifest    "${GEN_FOLDER}/win32.manifest")
	# so win32_resource can find the icon...
	configure_file("${CMAKE_SOURCE_DIR}/resources/mmcs.ico" "${GEN_FOLDER}/mmcs.ico" COPYONLY)
	list(APPEND mmcs_libs Comctl32 WS2_32 ntdll)
endif()

# Only executes in cmake's configure step...
#execute_process(COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${GEN_FOLDER}/Changelog.zip" --format=zip "${CMAKE_SOURCE_DIR}/Changelog.txt")
#execute_process(COMMAND ${CMAKE_COMMAND} -DGEN_FOLDER=${GEN_FOLDER} -P "${CMAKE_SOURCE_DIR}/cmake/ChangelogHandler.cmake")

#list(FILTER mmcs_sources EXCLUDE REGEX "README\\.md") # bye bye
add_executable(mmcs WIN32 ${mmcs_sources})
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT mmcs)
target_link_libraries(mmcs ${mmcs_libs})
