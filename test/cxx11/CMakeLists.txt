yk_cats_add_test_suite(11 main.cpp)

yk_cats_add_compile_fail_test(
    11 reserved_bit invalid_law_set.cpp
    YK_CATS_TEST_RESERVED_BIT
    "law_set uses a reserved bit"
)
yk_cats_add_compile_fail_test(
    11 selective_without_idempotent invalid_law_set.cpp
    YK_CATS_TEST_SELECTIVE_WITHOUT_IDEMPOTENT
    "selective implies idempotent"
)
yk_cats_add_compile_fail_test(
    11 associative_divisible_without_unital invalid_law_set.cpp
    YK_CATS_TEST_ASSOCIATIVE_DIVISIBLE_WITHOUT_UNITAL
    "associative and divisible imply unital"
)
