cmake_minimum_required(VERSION 3.14)
project(DeskBreezeWebView)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
      main.cpp
      embedded_assets.cpp
)

# 1) Define the target first
add_executable(${PROJECT_NAME} ${SOURCES})

# 2) Platform-specific setup
if(UNIX AND NOT APPLE)
  # Linux - WebKitGTK
  find_package(PkgConfig REQUIRED)

  # Try 4.1 first (Ubuntu 24.04+), fall back to 4.0
  pkg_check_modules(WEBKIT2 QUIET webkit2gtk-4.1)
  if(NOT WEBKIT2_FOUND)
    pkg_check_modules(WEBKIT2 REQUIRED webkit2gtk-4.0)
  endif()
  pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

  # Prefer proper package discovery for sqlite
  find_package(SQLite3 REQUIRED)

  target_include_directories(${PROJECT_NAME} PRIVATE
    ${GTK3_INCLUDE_DIRS}
    ${WEBKIT2_INCLUDE_DIRS}
  )
  target_link_directories(${PROJECT_NAME} PRIVATE
    ${GTK3_LIBRARY_DIRS}
    ${WEBKIT2_LIBRARY_DIRS}
  )
  # CFLAGS_OTHER are compiler options, not preprocessor defines
  target_compile_options(${PROJECT_NAME} PRIVATE
    ${GTK3_CFLAGS_OTHER}
    ${WEBKIT2_CFLAGS_OTHER}
  )
  target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GTK3_LIBRARIES}
    ${WEBKIT2_LIBRARIES}
    SQLite::SQLite3
    pthread
  )

elseif(APPLE)
  # macOS - WKWebView
  find_library(COCOA_LIBRARY Cocoa REQUIRED)
  find_library(WEBKIT_LIBRARY WebKit REQUIRED)
  find_package(SQLite3 REQUIRED)

  target_link_libraries(${PROJECT_NAME} PRIVATE
    ${COCOA_LIBRARY}
    ${WEBKIT_LIBRARY}
    SQLite::SQLite3
    pthread
  )

  # Use Objective-C++ for sources that touch Cocoa/WebKit
  # (rename to .mm ideally; otherwise force the flag:)
  set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++")

elseif(WIN32)
  # Windows - WebView2 (prefer vcpkg config package if available)
  find_package(SQLite3 REQUIRED)

  # If using vcpkg:
  # find_package(Microsoft.Web.WebView2 CONFIG REQUIRED)
  # target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::WebView2Loader)

  # If you vendor the SDK locally:
  target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/WebView2/build/native/include"
  )
  target_link_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/WebView2/build/native/x64"
  )
  target_link_libraries(${PROJECT_NAME} PRIVATE
    WebView2LoaderStatic
    SQLite::SQLite3
    ole32 oleaut32 user32 gdi32
  )
endif()
target_include_directories(DeskBreezeWebView PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})