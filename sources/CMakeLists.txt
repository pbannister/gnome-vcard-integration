#
#	CMakeLists.txt for the GNOME VCARD Integration reader, renderer, and tools.
#
#	The build tree is generated output and is written to dataflow.out/build/.
#	Warnings are treated as errors per prompts/flavors/02-cpp-conventions.md.
#
cmake_minimum_required(VERSION 3.16)
project(gnome_vcard_integration LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Werror)
include_directories("${CMAKE_BINARY_DIR}/generated")

find_package(PkgConfig QUIET)

add_library(version_info STATIC version/version_info.cpp)
target_include_directories(version_info PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(file_util STATIC util/file_io.cpp util/process.cpp)
target_include_directories(file_util PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(vcard_reader STATIC vcard/vcard_reader.cpp)
target_include_directories(vcard_reader PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(vcard_reader PUBLIC file_util)

add_library(contact_render STATIC render/contact_render.cpp)
target_include_directories(contact_render PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(contact_render PUBLIC vcard_reader file_util)

add_library(integration STATIC integration/handler_integrator.cpp)
target_include_directories(integration PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(integration PUBLIC file_util version_info)

# The viewer's non-GTK content: the Discovered command/output capture and the
# About identity.  Kept out of the GTK source so it can be tested everywhere.
add_library(viewer_content STATIC tools/viewer_content.cpp)
target_include_directories(viewer_content PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(viewer_content PUBLIC file_util version_info)

add_executable(vcard-inspect tools/vcard_inspect.cpp)
target_link_libraries(vcard-inspect PRIVATE vcard_reader file_util version_info)

add_executable(vcard-render tools/vcard_render.cpp)
target_link_libraries(vcard-render PRIVATE contact_render vcard_reader file_util version_info)

add_executable(vcard-integrate tools/vcard_integrate.cpp)
target_link_libraries(vcard-integrate PRIVATE integration file_util version_info)

file(COPY tools/icons DESTINATION "${CMAKE_BINARY_DIR}")

# The graphical viewer is a GTK4 program. It is built only where the GTK4
# development files are present; the command-line tools work without it, and
# `vcard-integrate handle` then prints the equivalent report instead.
if(PkgConfig_FOUND)
    pkg_check_modules(GTK4 IMPORTED_TARGET gtk4)
endif()
if(TARGET PkgConfig::GTK4)
    # The focus/selection clearing is a small GTK helper so the viewer and its
    # regression test use the same code.
    add_library(viewer_focus STATIC tools/viewer_focus.cpp)
    target_include_directories(viewer_focus PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(viewer_focus PUBLIC PkgConfig::GTK4)

    add_executable(vcard-viewer tools/vcard_viewer_ui.cpp)
    target_link_libraries(vcard-viewer PRIVATE PkgConfig::GTK4 contact_render vcard_reader
        viewer_content viewer_focus file_util version_info)

    add_executable(viewer-tabs-test "${CMAKE_CURRENT_SOURCE_DIR}/../tests/viewer_tabs_test.cpp")
    target_link_libraries(viewer-tabs-test PRIVATE viewer_focus)
else()
    message(STATUS "GTK4 development files not found: vcard-viewer is not built")
endif()

# --- tests -------------------------------------------------------------------
# The C++ tests are built here and run by the shell tests in tests/.
add_executable(vcard-reader-test "${CMAKE_CURRENT_SOURCE_DIR}/../tests/vcard_reader_test.cpp")
target_link_libraries(vcard-reader-test PRIVATE vcard_reader file_util)

add_executable(viewer-content-test "${CMAKE_CURRENT_SOURCE_DIR}/../tests/viewer_content_test.cpp")
target_link_libraries(viewer-content-test PRIVATE viewer_content)

add_executable(contact-render-test "${CMAKE_CURRENT_SOURCE_DIR}/../tests/contact_render_test.cpp")
target_link_libraries(contact-render-test PRIVATE contact_render vcard_reader)

add_executable(handler-integrator-test
    "${CMAKE_CURRENT_SOURCE_DIR}/../tests/handler_integrator_test.cpp")
target_link_libraries(handler-integrator-test PRIVATE integration)
