#include <yk/cats.hpp>

#if YK_CATS_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <climits>
#include <functional>

namespace {

namespace cats = yk::cats;
namespace instances = yk::cats::instances;
namespace structures = yk::cats::structures;

template<class Op, class T, class Instances>
using declares = cats::detail::declares_laws<cats::operation_laws<Op, T, Instances>>;

template<class Op, class T, class Instances>
constexpr cats::law_set laws_of()
{
  return cats::operation_laws<Op, T, Instances>::value;
}

struct concatenation {};

struct max_op {
  constexpr unsigned operator()(unsigned a, unsigned b) const noexcept { return a < b ? b : a; }
};

// Answers through member queries, which take precedence over the traits.
struct queried {
  constexpr unsigned query(cats::identity_element_t<unsigned>) const noexcept { return 1; }
  constexpr unsigned query(cats::inverse_t, unsigned x) const noexcept { return x + 1; }
};

struct generic_query {
  template<class T>
  constexpr T query(cats::identity_element_t<T>) const noexcept
  {
    return T(7);
  }
};

struct extended {
  using fallback = instances::lenient;
};

}  // namespace

namespace yk {
namespace cats {

template<>
struct operation_laws<concatenation, int, instances::strict> : std::integral_constant<law_set, structures::monoid> {};

template<>
struct operation_laws<max_op, unsigned, instances::strict> : std::integral_constant<law_set, structures::semilattice> {};

// strengthens the declaration of strict
template<>
struct operation_laws<max_op, unsigned, extended> : std::integral_constant<law_set, structures::bounded_semilattice | laws::selective> {};

template<>
struct identity_element_traits<queried, unsigned> {
  static constexpr unsigned identity_element(queried const&) noexcept { return 2; }
};

template<>
struct inverse_traits<queried, unsigned> {
  static constexpr unsigned inverse(queried const&, unsigned x) noexcept { return x + 2; }
};

}  // namespace cats
}  // namespace yk

TEST_CASE("strict declares exact laws only")
{
  STATIC_REQUIRE(laws_of<std::plus<unsigned>, unsigned, instances::strict>() == structures::commutative_group);
  STATIC_REQUIRE(laws_of<std::multiplies<unsigned long long>, unsigned long long, instances::strict>() == structures::commutative_monoid);
  STATIC_REQUIRE(laws_of<std::bit_and<unsigned long>, unsigned long, instances::strict>() == structures::bounded_semilattice);
  STATIC_REQUIRE(laws_of<std::bit_or<unsigned>, unsigned, instances::strict>() == structures::bounded_semilattice);
  STATIC_REQUIRE(laws_of<std::bit_xor<unsigned>, unsigned, instances::strict>() == structures::commutative_group);
  STATIC_REQUIRE(laws_of<std::bit_and<bool>, bool, instances::strict>() == structures::bounded_semilattice);
  STATIC_REQUIRE(laws_of<std::bit_xor<bool>, bool, instances::strict>() == structures::commutative_group);

  // signed overflow is undefined
  STATIC_REQUIRE(!declares<std::plus<int>, int, instances::strict>::value);
  // floating-point addition is not associative
  STATIC_REQUIRE(!declares<std::plus<double>, double, instances::strict>::value);
  // unsigned short is promoted to int, whose multiplication can overflow
  STATIC_REQUIRE(!declares<std::multiplies<unsigned short>, unsigned short, instances::strict>::value);
  STATIC_REQUIRE(!declares<std::bit_and<int>, int, instances::strict>::value);
  STATIC_REQUIRE(!declares<std::plus<bool>, bool, instances::strict>::value);
}

TEST_CASE("lenient adds signed and floating-point arithmetic")
{
  STATIC_REQUIRE(laws_of<std::plus<int>, int, instances::lenient>() == structures::commutative_group);
  STATIC_REQUIRE(laws_of<std::multiplies<double>, double, instances::lenient>() == structures::commutative_monoid);
  STATIC_REQUIRE(laws_of<std::plus<unsigned short>, unsigned short, instances::lenient>() == structures::commutative_group);
  STATIC_REQUIRE(laws_of<std::bit_and<int>, int, instances::lenient>() == structures::bounded_semilattice);

  // falls back to strict
  STATIC_REQUIRE(laws_of<std::plus<unsigned>, unsigned, instances::lenient>() == structures::commutative_group);
  STATIC_REQUIRE(laws_of<std::bit_or<bool>, bool, instances::lenient>() == structures::bounded_semilattice);

  STATIC_REQUIRE(!declares<std::plus<bool>, bool, instances::lenient>::value);
  STATIC_REQUIRE(!declares<std::minus<int>, int, instances::lenient>::value);
}

TEST_CASE("the operation and the carrier must agree")
{
  STATIC_REQUIRE(!declares<std::plus<int>, long, instances::lenient>::value);
  STATIC_REQUIRE(!declares<std::plus<unsigned>, int, instances::lenient>::value);
}

TEST_CASE("user declarations and user instance sets")
{
  STATIC_REQUIRE(laws_of<concatenation, int, instances::strict>() == structures::monoid);
  STATIC_REQUIRE(laws_of<concatenation, int, instances::lenient>() == structures::monoid);
  STATIC_REQUIRE(laws_of<concatenation, int, extended>() == structures::monoid);
  STATIC_REQUIRE(!declares<concatenation, long, instances::strict>::value);

  STATIC_REQUIRE(laws_of<max_op, unsigned, instances::lenient>() == structures::semilattice);
  STATIC_REQUIRE(laws_of<max_op, unsigned, extended>() == (structures::bounded_semilattice | yk::cats::laws::selective));
  STATIC_REQUIRE(laws_of<std::plus<int>, int, extended>() == structures::commutative_group);
}

TEST_CASE("operation_structure_tag dispatches on the declared laws")
{
  STATIC_REQUIRE(std::is_same<cats::operation_structure_tag<std::plus<unsigned>, unsigned>, cats::structure_tag<structures::commutative_group>>::value);
  STATIC_REQUIRE(
      std::is_same<cats::operation_structure_tag<std::plus<int>, int, instances::lenient>, cats::structure_tag<structures::commutative_group>>::value
  );
}

TEST_CASE("identity elements and inverses of the standard operations")
{
  STATIC_REQUIRE(cats::identity_element<unsigned>(std::plus<unsigned>{}) == 0u);
  STATIC_REQUIRE(cats::identity_element<int>(std::multiplies<int>{}) == 1);
  STATIC_REQUIRE(cats::identity_element<unsigned>(std::bit_and<unsigned>{}) == UINT_MAX);
  STATIC_REQUIRE(cats::identity_element<unsigned>(std::bit_or<unsigned>{}) == 0u);
  STATIC_REQUIRE(cats::identity_element<bool>(std::bit_and<bool>{}));
  STATIC_REQUIRE(!cats::identity_element<bool>(std::bit_xor<bool>{}));

  STATIC_REQUIRE(cats::inverse(std::plus<unsigned>{}, 1u) == UINT_MAX);
  STATIC_REQUIRE(cats::inverse(std::plus<int>{}, 3) == -3);
  STATIC_REQUIRE(cats::inverse(std::bit_xor<unsigned>{}, 5u) == 5u);
}

TEST_CASE("member queries take precedence over the traits")
{
  STATIC_REQUIRE(cats::identity_element<unsigned>(queried{}) == 1u);
  STATIC_REQUIRE(cats::inverse(queried{}, 3u) == 4u);

  STATIC_REQUIRE(cats::identity_element<unsigned>(generic_query{}) == 7u);
  STATIC_REQUIRE(cats::identity_element<long>(generic_query{}) == 7l);
}

TEST_CASE("stateless overloads of identity_element and inverse")
{
  STATIC_REQUIRE(cats::identity_element<unsigned, std::plus<unsigned>>() == 0u);
  STATIC_REQUIRE(cats::identity_element<unsigned, std::bit_and<unsigned>>() == UINT_MAX);
  STATIC_REQUIRE(cats::inverse<std::plus<unsigned>>(1u) == UINT_MAX);

  // a default-constructed operation still answers member queries
  STATIC_REQUIRE(cats::identity_element<unsigned, queried>() == 1u);
  STATIC_REQUIRE(cats::inverse<queried>(3u) == 4u);
}
