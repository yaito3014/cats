cmake_minimum_required(VERSION 3.24)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    if(NOT DEFINED CMAKE_PROJECT_TOP_LEVEL_INCLUDES)
        set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES
            "${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies.cmake"
        )
    endif()
endif()

project(yk-cats VERSION 0.0.1 LANGUAGES CXX)

add_library(yk_cats STATIC)

target_compile_features(yk_cats PUBLIC cxx_std_11)

file(GLOB_RECURSE YK_CATS_PUBLIC_HEADERS CONFIGURE_DEPENDS include/*.hpp)
file(GLOB_RECURSE YK_CATS_PRIVATE_HEADERS CONFIGURE_DEPENDS src/*.hpp)
file(GLOB_RECURSE YK_CATS_SOURCES CONFIGURE_DEPENDS src/*.cpp)

target_sources(
    yk_cats
    PUBLIC
        FILE_SET yk_cats_public_headers
            TYPE HEADERS
            BASE_DIRS include
            FILES ${YK_CATS_PUBLIC_HEADERS}
)

target_sources(
    yk_cats
    PRIVATE
        FILE_SET yk_cats_private_headers
            TYPE HEADERS
            BASE_DIRS src
            FILES ${YK_CATS_PRIVATE_HEADERS}
)

target_sources(yk_cats PRIVATE ${YK_CATS_SOURCES})

set_target_properties(yk_cats PROPERTIES EXPORT_NAME cats)
add_library(yk::cats ALIAS yk_cats)

option(YK_CATS_BUILD_TESTING "build test" ${PROJECT_IS_TOP_LEVEL})
if(YK_CATS_BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

option(
    YK_CATS_INSTALL
    "generate install rules for yk-cats"
    ${PROJECT_IS_TOP_LEVEL}
)

if(YK_CATS_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set(YK_CATS_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/yk-cats)

    install(
        TARGETS yk_cats
        EXPORT yk-cats-targets
        FILE_SET yk_cats_public_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        EXPORT yk-cats-targets
        FILE yk-cats-targets.cmake
        NAMESPACE yk::
        DESTINATION ${YK_CATS_INSTALL_CMAKEDIR}
    )

    configure_package_config_file(
        cmake/yk-cats-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/yk-cats-config.cmake
        INSTALL_DESTINATION ${YK_CATS_INSTALL_CMAKEDIR}
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/yk-cats-config-version.cmake
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/yk-cats-config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/yk-cats-config-version.cmake
        DESTINATION ${YK_CATS_INSTALL_CMAKEDIR}
    )
endif()
