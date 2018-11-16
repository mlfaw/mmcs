
cmake_minimum_required(VERSION 3.0)
project(mmcs_superbuild LANGUAGES C CXX)

if (POLICY CMP0074)
	cmake_policy(SET CMP0074 NEW) # for find_package(ZLIB) to use ZLIB_ROOT
endif()

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

set(GEN_FOLDER_BASE "${CMAKE_CURRENT_BINARY_DIR}/_generated")
include_directories("${GEN_FOLDER_BASE}")
set(GEN_FOLDER "${GEN_FOLDER_BASE}/generated")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${GEN_FOLDER_BASE}")
include(CreateCResources)
include(MSVCParallelBuild)
#include(MSVCStaticLibs)

set(MY_LIBRESSL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/libressl-2.8.2/include")
set(MY_LIBRESSL_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/libressl")
configure_file("${CMAKE_SOURCE_DIR}/cmake/FindOpenSSL.in.cmake" "${GEN_FOLDER_BASE}/FindOpenSSL.cmake" @ONLY)
# Includes our static-built-libressl in place of OpenSSL
find_package(OpenSSL REQUIRED)

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
	"${CMAKE_SOURCE_DIR}/thirdparty/sqlite-amalgamation-3250200/sqlite3.c"
	"${CMAKE_SOURCE_DIR}/thirdparty/sqlite-amalgamation-3250200/sqlite3.h"
	#"src/small_zip.c"
	#"src/small_zip.h"
)

# TODO: https://www.sqlite.org/compile.html
add_definitions(
	# if the load extension is ever to be used though then Linux needs the dl lib
	-DSQLITE_OMIT_LOAD_EXTENSION=1
)

if (WIN32)
	add_definitions(
		-D_UNICODE
		-DUNICODE
		-D_CRT_SECURE_NO_WARNINGS
		-DWINVER=0x0601
		-D_WIN32_WINNT=0x0601
		-DSQLITE_WIN32_GETVERSIONEX=0 # something something deprecation warnings on GetVersionEx[AW]
		-DGDIPVER=0x0110 # GDI+ 1.1 for the Graphics->DrawImage() func we use
		-DMMCS_USE_GDIP=0 # I don't think I'll use GDI+ but I'll keep the code and disabled with this flag for now...
	)
	list(APPEND mmcs_libs d3d11 d2d1 dxgi dwrite)
	file(GLOB win_sources "src/msw_*" "src/win32_*")
	list(APPEND mmcs_sources
		${win_sources}
		"${GEN_FOLDER}/win32_resource.h"
		"${GEN_FOLDER}/win32_resource.rc"
		"${GEN_FOLDER}/win32.manifest"
	)
	configure_file(src/gen_win32_resource.in.h  "${GEN_FOLDER}/win32_resource.h")
	configure_file(src/gen_win32_resource.in.rc "${GEN_FOLDER}/win32_resource.rc")
	configure_file(src/gen_win32.in.manifest    "${GEN_FOLDER}/win32.manifest")
	# so win32_resource can find the icon...
	configure_file("${CMAKE_SOURCE_DIR}/resources/mmcs.ico" "${GEN_FOLDER}/mmcs.ico" COPYONLY)

	list(APPEND mmcs_libs Comctl32 WS2_32 ntdll gdiplus)
else()
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	list(APPEND mmcs_libs Threads::Threads)
endif()

include(ExternalProject)
if (MSVC)
	set(__BUILD_TYPE_PREFIX "${CMAKE_BUILD_TYPE}/")
else()
	set(__BUILD_TYPE_PREFIX "")
endif()

set(CURL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/curl-7.62.0")
set(CURL_BUILD_DIR "${CMAKE_BINARY_DIR}/curl/src/curl-build/lib")
if (CMAKE_BUILD_TYPE MATCHES Debug)
	set(CURL_LIB_POSTFIX "-d")
	if (MSVC)
		set(ZLIB_LIB_POSTFIX "d")
	endif()
else()
	set(CURL_LIB_POSTFIX "")
endif()
set(CURL_CMAKE_ARGS
	-DBUILD_CURL_EXE:BOOL=OFF
	-DBUILD_SHARED_LIBS:BOOL=OFF
	-DHTTP_ONLY:BOOL=ON
	-DCMAKE_USE_LIBSSH2:BOOL=OFF
	-DCMAKE_USE_OPENSSL:BOOL=ON
	-DCURL_ZLIB:BOOL=OFF
	-DENABLE_MANUAL:BOOL=OFF
	-DCURL_CA_BUNDLE:STRING=none
	-DCMAKE_MODULE_PATH:STRING=${CMAKE_MODULE_PATH}
	-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
	-DBUILD_TESTING:BOOL=OFF
)
ExternalProject_Add(
	curl
	PREFIX curl
	INSTALL_COMMAND ""
	SOURCE_DIR "${CURL_SOURCE_DIR}"
	CMAKE_CACHE_ARGS "${CURL_CMAKE_ARGS}"
)
add_library(mylibcurl STATIC IMPORTED)
set_target_properties(mylibcurl PROPERTIES IMPORTED_LOCATION "${CURL_BUILD_DIR}/${__BUILD_TYPE_PREFIX}libcurl${CURL_LIB_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/zlib-1.2.11")
if (UNIX)
	set(ZLIB_LIBRARY "${CMAKE_BINARY_DIR}/zlib/${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
	set(ZLIB_LIBRARY "${CMAKE_BINARY_DIR}/zlib/${__BUILD_TYPE_PREFIX}${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic${ZLIB_LIB_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()
find_package(ZLIB)

include_directories(
	"${ZLIB_INCLUDE_DIRS}"
	"${OPENSSL_INCLUDE_DIR}"
	"${CURL_SOURCE_DIR}/include"
	"${CURL_BUILD_DIR}"
)

list(APPEND mmcs_libs
	OpenSSL::Crypto
	OpenSSL::SSL
	ZLIB::ZLIB
	"${LIBRESSL_TLS_LIBRARY}"
	mylibcurl
)

# Only executes in cmake's configure step...
#execute_process(COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${GEN_FOLDER}/Changelog.zip" --format=zip "${CMAKE_SOURCE_DIR}/Changelog.txt")
#execute_process(COMMAND ${CMAKE_COMMAND} -DGEN_FOLDER=${GEN_FOLDER} -P "${CMAKE_SOURCE_DIR}/cmake/ChangelogHandler.cmake")

#list(FILTER mmcs_sources EXCLUDE REGEX "README\\.md") # bye bye
add_executable(mmcs WIN32 ${mmcs_sources})
add_dependencies(mmcs curl) # require the externalproject to be built first
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT mmcs)
target_link_libraries(mmcs ${mmcs_libs})
