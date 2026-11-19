#include <yk/cats.hpp>

#include <catch2/catch_test_macros.hpp>

#include <functional>

namespace {

namespace cats = yk::cats;
namespace instances = yk::cats::instances;
namespace structures = yk::cats::structures;

template<class Op, class T, class Instances>
using declares = cats::detail::declares_laws<cats::operation_laws<Op, T, Instances>>;

}  // namespace

TEST_CASE("transparent operators")
{
  STATIC_REQUIRE(cats::operation_laws<std::plus<>, unsigned, instances::strict>::value == structures::commutative_group);
  STATIC_REQUIRE(cats::operation_laws<std::bit_or<>, unsigned long, instances::strict>::value == structures::bounded_semilattice);
  STATIC_REQUIRE(cats::operation_laws<std::multiplies<>, double, instances::lenient>::value == structures::commutative_monoid);

  // short + short is int, so the operation is not closed on short
  STATIC_REQUIRE(!declares<std::plus<>, short, instances::lenient>::value);
  STATIC_REQUIRE(!declares<std::bit_and<>, bool, instances::lenient>::value);

  STATIC_REQUIRE(cats::identity_element<unsigned>(std::bit_and<>{}) == ~0u);
}
