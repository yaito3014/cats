#include <yk/cats.hpp>

#if YK_CATS_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

#include <climits>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

namespace {

namespace cats = yk::cats;
namespace instances = yk::cats::instances;
namespace laws = yk::cats::laws;
namespace structures = yk::cats::structures;

struct constexpr_plus {
  constexpr unsigned operator()(unsigned a, unsigned b) const noexcept { return a + b; }
};

// Addition modulo a runtime modulus, whose inverse depends on the operation's
// state.
struct modular_plus {
  unsigned modulus;
  constexpr unsigned operator()(unsigned a, unsigned b) const noexcept { return (a + b) % modulus; }
  constexpr unsigned query(cats::identity_element_t<unsigned>) const noexcept { return 0; }
  constexpr unsigned query(cats::inverse_t, unsigned x) const noexcept { return (modulus - x) % modulus; }
};

// Counts its applications, so tests can tell which algorithm was chosen.
template<class Op>
struct counting {
  Op op;
  int* count;
  unsigned operator()(unsigned a, unsigned b) const
  {
    ++*count;
    return op(a, b);
  }
};

// Declares any law set, to check that power dispatches unambiguously on it.
// It is never applied.
template<cats::law_set Laws>
struct law_op {
  constexpr unsigned operator()(unsigned a, unsigned) const noexcept { return a; }
};

}  // namespace

namespace yk {
namespace cats {

template<>
struct operation_laws<constexpr_plus, unsigned, instances::strict> : std::integral_constant<law_set, structures::commutative_monoid> {};

template<>
struct operation_laws<modular_plus, unsigned, instances::strict> : std::integral_constant<law_set, structures::commutative_group> {};

template<>
struct identity_element_traits<constexpr_plus, unsigned> {
  static constexpr unsigned identity_element(constexpr_plus const&) noexcept { return 0; }
};

template<>
struct operation_laws<counting<std::plus<unsigned>>, unsigned, instances::strict> : std::integral_constant<law_set, structures::semigroup> {};

template<>
struct operation_laws<counting<std::bit_or<unsigned>>, unsigned, instances::strict> : std::integral_constant<law_set, structures::band> {};

template<>
struct operation_laws<std::plus<std::string>, std::string, instances::strict> : std::integral_constant<law_set, structures::monoid> {};

template<>
struct identity_element_traits<std::plus<std::string>, std::string> {
  static std::string identity_element(std::plus<std::string> const&) { return std::string(); }
};

template<law_set Laws>
struct operation_laws<law_op<Laws>, unsigned, instances::strict> : std::integral_constant<law_set, Laws> {};

template<law_set Laws>
struct identity_element_traits<law_op<Laws>, unsigned> {
  static constexpr unsigned identity_element(law_op<Laws> const&) noexcept { return 0; }
};

template<law_set Laws>
struct inverse_traits<law_op<Laws>, unsigned> {
  static constexpr unsigned inverse(law_op<Laws> const&, unsigned x) noexcept { return x; }
};

}  // namespace cats
}  // namespace yk

namespace {

template<cats::law_set Laws, class = void>
struct dispatches : std::false_type {};

template<cats::law_set Laws>
struct dispatches<Laws, decltype(void(cats::detail::power_impl(cats::make_structure_tag<Laws>(), std::declval<law_op<Laws> const&>(), 0u, 1)))>
    : std::true_type {};

// Defers naming dispatches<Laws> so that invalid sets never reach the checked structure_tag.
template<cats::law_set Laws>
constexpr bool dispatches_if_applicable() noexcept
{
  return std::conditional < cats::is_valid_law_set(Laws) && (Laws & laws::associative) != 0, dispatches<Laws>, std::true_type > ::type::value;
}

constexpr bool all_of() noexcept { return true; }

template<class... Bools>
constexpr bool all_of(bool head, Bools... tail) noexcept
{
  return head && all_of(tail...);
}

template<cats::law_set... Sets>
constexpr bool all_dispatch(cats::detail::integer_sequence<cats::law_set, Sets...>) noexcept
{
  return all_of(dispatches_if_applicable<Sets>()...);
}

}  // namespace

TEST_CASE("power of a monoid")
{
  STATIC_REQUIRE(cats::power(constexpr_plus{}, 3u, 5) == 15u);
  STATIC_REQUIRE(cats::power(constexpr_plus{}, 3u, 0) == 0u);
  STATIC_REQUIRE(cats::power(constexpr_plus{}, 3u, 1u) == 3u);

  REQUIRE(cats::power(std::multiplies<unsigned long long>{}, 3ull, 20) == 3486784401ull);
  REQUIRE(cats::power(std::multiplies<unsigned>{}, 7u, 0) == 1u);

  // non-commutative, so the order of the operands matters
  REQUIRE(cats::power(std::plus<std::string>{}, std::string("ab"), 3) == "ababab");
  REQUIRE(cats::power(std::plus<std::string>{}, std::string("ab"), 0).empty());
}

TEST_CASE("power of a group accepts negative exponents")
{
  REQUIRE(cats::power(std::plus<unsigned>{}, 3u, -2) == 0u - 6u);
  REQUIRE(cats::power(std::bit_xor<unsigned>{}, 5u, -3) == 5u);
  REQUIRE(cats::power(std::bit_xor<unsigned>{}, 5u, 4) == 0u);
  REQUIRE(cats::power(std::plus<unsigned char>{}, static_cast<unsigned char>(1), static_cast<signed char>(-128)) == 128);
}

TEST_CASE("power uses the member queries of a stateful operation")
{
  STATIC_REQUIRE(cats::power(modular_plus{7}, 3u, 4) == 5u);
  STATIC_REQUIRE(cats::power(modular_plus{7}, 3u, 0) == 0u);
  STATIC_REQUIRE(cats::power(modular_plus{7}, 3u, -2) == 1u);
}

TEST_CASE("power with the lenient instances")
{
  REQUIRE(cats::power(std::multiplies<int>{}, 2, 10) == 1024);
  REQUIRE(cats::power(std::plus<int>{}, 4, -3) == -12);
  REQUIRE(cats::power(std::plus<double>{}, 0.5, 4) == 2.0);
  REQUIRE(cats::power(std::plus<int>{}, 1, INT_MIN) == INT_MIN);
}

TEST_CASE("power of an idempotent operation")
{
  REQUIRE(cats::power(std::bit_or<unsigned>{}, 5u, 100) == 5u);
  REQUIRE(cats::power(std::bit_or<unsigned>{}, 5u, 0) == 0u);
  REQUIRE(cats::power(std::bit_and<bool>{}, false, 0));
}

TEST_CASE("power picks the algorithm from the laws")
{
  int count = 0;

  // associativity allows repeated squaring
  REQUIRE(cats::power(counting<std::plus<unsigned>>{{}, &count}, 1u, 1000) == 1000u);
  REQUIRE(count <= 2 * 10);

  // idempotence makes every positive power the element itself
  count = 0;
  REQUIRE(cats::power(counting<std::bit_or<unsigned>>{{}, &count}, 7u, 1000) == 7u);
  REQUIRE(count == 0);
}

TEST_CASE("stateless overloads of power")
{
  STATIC_REQUIRE(cats::power<constexpr_plus>(3u, 5) == 15u);
  STATIC_REQUIRE(cats::power<constexpr_plus>(3u, 0) == 0u);

  REQUIRE(cats::power<std::multiplies<int>>(2, 10) == 1024);
  REQUIRE(cats::power<std::plus<unsigned>, instances::strict>(3u, -2) == 0u - 6u);
  REQUIRE(cats::power<std::plus<std::string>>(std::string("ab"), 3) == "ababab");
}

TEST_CASE("power dispatches unambiguously on every valid associative law set")
{
  STATIC_REQUIRE(all_dispatch(cats::detail::make_integer_sequence<cats::law_set, 64>{}));
}
