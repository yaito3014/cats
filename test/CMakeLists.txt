find_package(Catch2 3 REQUIRED)

function(yk_cats_add_test test_name)
    set(target_name yk_cats_${test_name}_test)
    add_executable(${target_name} ${test_name}.cpp)
    target_link_libraries(${target_name} PRIVATE yk::cats)
    target_link_libraries(${target_name} PRIVATE Catch2::Catch2WithMain)
    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

yk_cats_add_test(main)
