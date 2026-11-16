include(FetchContent)

# Fetches Catch2 in a function scope so that the variables set to configure it
# do not leak into the caller of find_package.
function(yk_cats_fetch_catch2 major_version)
    if(major_version EQUAL 2)
        # Catch2 v3 requires C++14, so the C++11 tests use v2.
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2
            GIT_TAG v2.13.10
            GIT_SHALLOW ON
        )

        # Catch2 v2 declares cmake_minimum_required(VERSION 3.5), so without
        # CMP0077 its option() would ignore this variable.
        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        set(CATCH_BUILD_STATIC_LIBRARY ON)
        FetchContent_MakeAvailable(Catch2)

        # Catch2 v2.13.10 uses std::nothrow without including <new>, which
        # libc++ does not provide transitively.
        include(CheckCXXSourceCompiles)
        check_cxx_source_compiles(
            "#include <cstddef>\n#ifndef _LIBCPP_VERSION\n#error not libc++\n#endif\nint main() {}"
            YK_CATS_STDLIB_IS_LIBCXX
        )
        if(YK_CATS_STDLIB_IS_LIBCXX)
            target_compile_options(Catch2 INTERFACE "SHELL:-include new")
        endif()
    else()
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2
            GIT_TAG v3.15.0
            GIT_SHALLOW ON
        )
        FetchContent_MakeAvailable(Catch2)
    endif()
endfunction()

macro(yk_cats_provide_dependency method package_name)
    if("${package_name}" STREQUAL "Catch2")
        # The tests call find_package(Catch2 <major> REQUIRED), so the
        # argument after the package name is the major version.
        yk_cats_fetch_catch2("${ARGV2}")
        set(Catch2_FOUND ON)
    endif()
endmacro()

cmake_language(
    SET_DEPENDENCY_PROVIDER yk_cats_provide_dependency
    SUPPORTED_METHODS FIND_PACKAGE
)
