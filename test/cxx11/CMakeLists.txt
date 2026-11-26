yk_cats_add_test_suite(11 main.cpp operation_laws.cpp power.cpp)

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
yk_cats_add_compile_fail_test(
    11 overload_declaration invalid_law_set.cpp
    YK_CATS_TEST_OVERLOAD_DECLARATION
    "selective implies idempotent"
)
yk_cats_add_compile_fail_test(
    11 undeclared_operation invalid_operation.cpp
    YK_CATS_TEST_UNDECLARED_OPERATION
    "no laws are declared"
)
yk_cats_add_compile_fail_test(
    11 strict_signed invalid_operation.cpp
    YK_CATS_TEST_STRICT_SIGNED
    "no laws are declared"
)
yk_cats_add_compile_fail_test(
    11 weakened_fallback invalid_operation.cpp
    YK_CATS_TEST_WEAKENED_FALLBACK
    "must not weaken"
)
yk_cats_add_compile_fail_test(
    11 missing_identity_element invalid_operation.cpp
    YK_CATS_TEST_MISSING_IDENTITY_ELEMENT
    "no identity element is declared"
)
yk_cats_add_compile_fail_test(
    11 missing_inverse invalid_operation.cpp
    YK_CATS_TEST_MISSING_INVERSE
    "no inverse is declared"
)
yk_cats_add_compile_fail_test(
    11 not_associative invalid_operation.cpp
    YK_CATS_TEST_NOT_ASSOCIATIVE
    "power requires an associative operation"
)
yk_cats_add_compile_fail_test(
    11 non_integer_exponent invalid_operation.cpp
    YK_CATS_TEST_NON_INTEGER_EXPONENT
    "power requires an integer exponent"
)
yk_cats_add_compile_fail_test(
    11 not_default_constructible invalid_operation.cpp
    YK_CATS_TEST_NOT_DEFAULT_CONSTRUCTIBLE
    "requires a default-constructible operation"
)
