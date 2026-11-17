#include <yk/cats.hpp>

#if YK_CATS_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <type_traits>

TEST_CASE("law_lattice subsumption")
{
  using yk::cats::detail::law_lattice;
  namespace structures = yk::cats::structures;

  // subsumption
  STATIC_REQUIRE(std::is_base_of<law_lattice<structures::magma>, law_lattice<structures::monoid>>::value);
  STATIC_REQUIRE(std::is_base_of<law_lattice<structures::loop>, law_lattice<structures::commutative_group>>::value);

  // incomparable
  STATIC_REQUIRE(!std::is_base_of<law_lattice<structures::band>, law_lattice<structures::monoid>>::value);
  STATIC_REQUIRE(!std::is_base_of<law_lattice<structures::monoid>, law_lattice<structures::band>>::value);
}

namespace {

using yk::cats::law_set;
using yk::cats::make_structure_tag;
using yk::cats::structure_tag;
namespace structures = yk::cats::structures;

constexpr int pick(structure_tag<structures::monoid>) { return 1; }
constexpr int pick(structure_tag<structures::band>) { return 2; }
constexpr int pick(structure_tag<structures::group>) { return 3; }

template<law_set Laws, class = void>
struct can_pick : std::false_type {};

template<law_set Laws>
struct can_pick<Laws, decltype(void(pick(make_structure_tag<Laws>())))> : std::true_type {};

}  // namespace

TEST_CASE("structure_tag overload resolution")
{
  STATIC_REQUIRE(pick(make_structure_tag<structures::commutative_monoid>()) == 1);
  STATIC_REQUIRE(pick(make_structure_tag<structures::semilattice>()) == 2);

  // group is more specific than monoid
  STATIC_REQUIRE(pick(make_structure_tag<structures::commutative_group>()) == 3);

  // monoid and band are incomparable, so the call is ambiguous
  STATIC_REQUIRE(!can_pick<structures::idempotent_monoid>::value);

  // no overload accepts a loop
  STATIC_REQUIRE(!can_pick<structures::loop>::value);
}
