option(YK_CATS_CI "for CI" OFF)

set(CMAKE_COLOR_DIAGNOSTICS ${YK_CATS_CI})

if(DEFINED CMAKE_CXX_STANDARD)
    set(YK_CATS_CXX_STANDARD ${CMAKE_CXX_STANDARD})
else()
    set(YK_CATS_CXX_STANDARD 11)
endif()

if(YK_CATS_CXX_STANDARD LESS 14)
    set(YK_CATS_CATCH2_MAJOR_VERSION 2)
else()
    set(YK_CATS_CATCH2_MAJOR_VERSION 3)
endif()

find_package(Catch2 ${YK_CATS_CATCH2_MAJOR_VERSION} REQUIRED)

# A compile feature is only a minimum, so without CXX_STANDARD the tests would
# build with the compiler's default standard (e.g. C++17 for GCC 15).
function(yk_cats_pin_cxx_standard target_name)
    set_target_properties(
        ${target_name}
        PROPERTIES
            CXX_STANDARD ${YK_CATS_CXX_STANDARD}
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )
endfunction()

if(TARGET Catch2WithMain)
    yk_cats_pin_cxx_standard(Catch2WithMain)
endif()

function(yk_cats_add_test_suite cxx_version)
    set(target_name yk_cats_cxx${cxx_version}_test)
    add_executable(${target_name} ${ARGN})
    yk_cats_pin_cxx_standard(${target_name})
    target_compile_features(${target_name} PRIVATE cxx_std_${cxx_version})
    target_compile_definitions(
        ${target_name}
        PRIVATE YK_CATS_CATCH2_MAJOR_VERSION=${YK_CATS_CATCH2_MAJOR_VERSION}
    )
    target_link_libraries(
        ${target_name}
        PRIVATE yk::cats Catch2::Catch2WithMain
    )
    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

# test/cxxNN holds the tests that require C++NN; build every suite up to the
# selected standard.
foreach(cxx_version IN ITEMS 11 14 17 20 23 26)
    if(cxx_version GREATER YK_CATS_CXX_STANDARD)
        break()
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cxx${cxx_version})
        add_subdirectory(cxx${cxx_version})
    endif()
endforeach()
