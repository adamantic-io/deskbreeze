
cmake_minimum_required(VERSION 3.14)
project(PetStoreQtSmart)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)
find_package(Python3 REQUIRED)

add_executable(PetStoreQtAllInOne
    main.cpp
    embedded_assets.cpp
)

target_include_directories(PetStoreQtAllInOne PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(PetStoreQtAllInOne PRIVATE http_server.hpp incbin.h)
target_link_libraries(PetStoreQtAllInOne Qt6::Core Qt6::Widgets Qt6::WebEngineWidgets sqlite3)
