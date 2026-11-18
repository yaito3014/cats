#include <yk/cats.hpp>

namespace laws = yk::cats::laws;

#if defined(YK_CATS_TEST_RESERVED_BIT)
auto const tag = yk::cats::make_structure_tag<yk::cats::law_set{1} << 6>();
#elif defined(YK_CATS_TEST_SELECTIVE_WITHOUT_IDEMPOTENT)
auto const tag = yk::cats::make_structure_tag<laws::associative | laws::selective>();
#elif defined(YK_CATS_TEST_ASSOCIATIVE_DIVISIBLE_WITHOUT_UNITAL)
auto const tag = yk::cats::make_structure_tag<laws::associative | laws::divisible>();
#elif defined(YK_CATS_TEST_OVERLOAD_DECLARATION)
void f(yk::cats::structure_tag<laws::associative | laws::selective>);
#endif
