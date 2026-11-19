#include <yk/cats.hpp>

#include <functional>

namespace cats = yk::cats;

namespace {

// a group, but neither its identity element nor its inverses are declared
struct add {
  unsigned operator()(unsigned a, unsigned b) const { return a + b; }
};

struct weakened {
  using fallback = cats::instances::strict;
};

}  // namespace

namespace yk {
namespace cats {

#if defined(YK_CATS_TEST_WEAKENED_FALLBACK)
template<>
struct operation_laws<std::plus<unsigned>, unsigned, weakened> : std::integral_constant<law_set, structures::commutative_monoid> {};
#endif

}  // namespace cats
}  // namespace yk

#if defined(YK_CATS_TEST_UNDECLARED_OPERATION)
cats::operation_structure_tag<std::minus<unsigned>, unsigned> const tag = nullptr;
#elif defined(YK_CATS_TEST_STRICT_SIGNED)
cats::operation_structure_tag<std::plus<int>, int, cats::instances::strict> const tag = nullptr;
#elif defined(YK_CATS_TEST_WEAKENED_FALLBACK)
cats::operation_structure_tag<std::plus<unsigned>, unsigned, weakened> const tag = nullptr;
#elif defined(YK_CATS_TEST_MISSING_IDENTITY_ELEMENT)
unsigned const result = cats::identity_element<unsigned>(add{});
#elif defined(YK_CATS_TEST_MISSING_INVERSE)
unsigned const result = cats::inverse(add{}, 1u);
#endif
