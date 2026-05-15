cmake_minimum_required(VERSION 3.10)

set(VER_MAJOR 1)
set(VER_MINOR 0)
set(VER_PATCH 0)
set(VER_BUILD 6)
set(APP_VERSION "${VER_MAJOR}.${VER_MINOR}.${VER_PATCH}.${VER_BUILD}")

set(COMPANY_NAME "Pharos Labs")
set(COMPANY_URL "https://github.com/pharos-labs")

project("PharosVersionChooser")

# Set C++ 20 Standard
set(CMAKE_CXX_STANDARD 20)

# Use for nice layout in IDE
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set versions in the resource so it shows in app properties
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/resources/version.rc.in
  ${CMAKE_CURRENT_LIST_DIR}/resources/version.rc
  @ONLY)

# Source Files
add_executable(DesignerVersionChooser WIN32
    src/DesignerVersionChooser.cpp
	src/DesignerVersionList.cpp
    resources/DesignerVersionChooser.rc
    resources/DesignerVersionChooser.exe.manifest
    )

target_include_directories(DesignerVersionChooser PRIVATE
    src
    resources
    )

target_compile_definitions(DesignerVersionChooser PRIVATE DESIGNERVERSIONCHOOSER)

# Source Files
add_executable(ExpressVersionChooser WIN32
    src/DesignerVersionChooser.cpp
	src/DesignerVersionList.cpp
    resources/DesignerVersionChooser.rc
    resources/DesignerVersionChooser.exe.manifest
    )

target_include_directories(ExpressVersionChooser PRIVATE
    src
    resources
    )

target_compile_definitions(ExpressVersionChooser PRIVATE EXPRESSVERSIONCHOOSER)

# Mosaic target
add_executable(MosaicVersionChooser WIN32
    src/DesignerVersionChooser.cpp
    src/DesignerVersionList.cpp
    resources/DesignerVersionChooser.rc
    resources/DesignerVersionChooser.exe.manifest
    )

target_include_directories(MosaicVersionChooser PRIVATE
    src
    resources
    )

target_compile_definitions(MosaicVersionChooser PRIVATE MOSAICVERSIONCHOOSER)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_UNICODE -DUNICODE)
    set_target_properties(DesignerVersionChooser PROPERTIES COMPILE_FLAGS "/Zc:wchar_t")
    set_target_properties(ExpressVersionChooser PROPERTIES COMPILE_FLAGS "/Zc:wchar_t")
    set_target_properties(MosaicVersionChooser PROPERTIES COMPILE_FLAGS "/Zc:wchar_t")
endif()

# The ComCtl32 library modernizes UI to vista style
target_link_libraries(DesignerVersionChooser PUBLIC Version ComCtl32)
target_link_libraries(ExpressVersionChooser PUBLIC Version ComCtl32)
target_link_libraries(MosaicVersionChooser PUBLIC Version ComCtl32)

# Override the default NSIS template with our own
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/install" ${CMAKE_MODULE_PATH})
set(CPACK_PACKAGE_VERSION ${APP_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${VER_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VER_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VER_PATCH})

# Installer using NSIS
include(CPack)

install(
        TARGETS
        DesignerVersionChooser
        DESTINATION "bin"
    )
