set (CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/.vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
cmake_minimum_required(VERSION 3.14)
project(DeskBreezeWebView)

#find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

add_custom_target(react_build ALL
    COMMAND npm install
    COMMAND npm run build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/frontend
    COMMENT "Building React app"
)

#add_custom_target(generate_assets ALL
#    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/generate_embedded_assets.py
#    DEPENDS react_build
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#    COMMENT "Generating embedded assets"
#)

add_subdirectory(app)
add_dependencies(${PROJECT_NAME} react_build)
#add_dependencies(${PROJECT_NAME} generate_assets)

include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "TGZ;ZIP")
include(CPack)
