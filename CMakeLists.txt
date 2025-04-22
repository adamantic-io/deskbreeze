
cmake_minimum_required(VERSION 3.14)
project(PetStoreAllInOneSmart)

add_custom_target(react_build ALL
    COMMAND npm install
    COMMAND npm run build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/frontend
    COMMENT "Building React app"
)

# add_custom_command(
#     OUTPUT ${CMAKE_SOURCE_DIR}/qt-app/embedded_assets.cpp
#     COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/generate_embedded_assets.py
#     DEPENDS react_build
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/scripts
# )
# 
# add_custom_target(generate_embedded
#     DEPENDS ${CMAKE_SOURCE_DIR}/qt-app/embedded_assets.cpp
# )

add_subdirectory(qt-app)
add_dependencies(PetStoreQtAllInOne react_build)
# add_dependencies(PetStoreQtAllInOne generate_embedded)

include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "TGZ;ZIP")
include(CPack)
