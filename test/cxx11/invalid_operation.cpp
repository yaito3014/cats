#include <yk/cats.hpp>

#include <functional>

namespace cats = yk::cats;

namespace {

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
#endif
