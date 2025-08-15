
cmake_minimum_required(VERSION 3.14)
project(DeskBreezeWebView)

add_custom_target(react_build ALL
    COMMAND npm install
    COMMAND npm run build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/frontend
    COMMENT "Building React app"
)

add_subdirectory(app)
add_dependencies(DeskBreezeWebView react_build)

include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "TGZ;ZIP")
include(CPack)
