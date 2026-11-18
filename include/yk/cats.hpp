#ifndef YK_ZZ_CATS_HPP
#define YK_ZZ_CATS_HPP

#include <type_traits>

#include <climits>
#include <cstddef>
#include <cstdint>

namespace yk {

namespace cats {

using law_set = std::uint_least32_t;

namespace detail {

template<class T, T... Is>
struct integer_sequence {
  using value_type = T;
  static constexpr std::size_t size() noexcept { return sizeof...(Is); }
};

template<class T, std::size_t N, T... Is>
struct make_integer_sequence_impl : make_integer_sequence_impl<T, N - 1, T(N - 1), Is...> {};

template<class T, T... Is>
struct make_integer_sequence_impl<T, 0, Is...> {
  using type = integer_sequence<T, Is...>;
};

template<class IntegerType, IntegerType Count>
using make_integer_sequence = typename make_integer_sequence_impl<IntegerType, Count>::type;

template<
    class BitmaskIntegralConstant,
    class IndexIntegralSequence = make_integer_sequence<unsigned, sizeof(typename BitmaskIntegralConstant::value_type) * CHAR_BIT>,
    class IntermediateIntegralSequence = integer_sequence<typename BitmaskIntegralConstant::value_type>>
struct decompose {};

template<class BitmaskType, BitmaskType Bitmask, unsigned Head, unsigned... Rest, BitmaskType... Intermediates>
struct decompose<std::integral_constant<BitmaskType, Bitmask>, integer_sequence<unsigned, Head, Rest...>, integer_sequence<BitmaskType, Intermediates...>> {
  using type = typename std::conditional<
      ((Bitmask >> Head) & 1) != 0,
      decompose<
          std::integral_constant<BitmaskType, Bitmask & ~(BitmaskType{1} << Head)>, integer_sequence<unsigned, Rest...>,
          integer_sequence<BitmaskType, Intermediates..., (BitmaskType{1} << Head)>>,
      decompose<std::integral_constant<BitmaskType, Bitmask>, integer_sequence<unsigned, Rest...>, integer_sequence<BitmaskType, Intermediates...>>>::type::
      type;
};

template<class BitmaskType, BitmaskType Bitmask, BitmaskType... Bits>
struct decompose<std::integral_constant<BitmaskType, Bitmask>, integer_sequence<unsigned>, integer_sequence<BitmaskType, Bits...>> {
  using type = integer_sequence<BitmaskType, Bits...>;
};

template<law_set Laws>
struct law_lattice;

template<law_set Laws, class DecomposedIntegralSequence>
struct law_lattice_bases {};

template<law_set Laws, law_set... Bits>
struct law_lattice_bases<Laws, integer_sequence<law_set, Bits...>> : virtual law_lattice<Laws & ~Bits>... {};

// law_lattice<S> is a virtual base of law_lattice<L> iff S is a subset of L.
template<law_set Laws>
struct law_lattice : law_lattice_bases<Laws, typename decompose<std::integral_constant<law_set, Laws>>::type> {};

}  // namespace detail

namespace laws {

constexpr law_set associative = law_set{1} << 0;
constexpr law_set unital = law_set{1} << 1;
constexpr law_set divisible = law_set{1} << 2;  // For every a and b, a * x = b and y * a = b have unique solutions x and y.
constexpr law_set commutative = law_set{1} << 3;
constexpr law_set idempotent = law_set{1} << 4;
constexpr law_set selective = law_set{1} << 5;  // a * b is either a or b; implies idempotent.

}  // namespace laws

namespace detail {

constexpr law_set known_laws = laws::associative | laws::unital | laws::divisible | laws::commutative | laws::idempotent | laws::selective;
constexpr law_set associative_divisible = laws::associative | laws::divisible;

constexpr bool uses_only_known_laws(law_set set) noexcept { return (set & ~known_laws) == 0; }

constexpr bool selective_implies_idempotent(law_set set) noexcept { return (set & laws::selective) == 0 || (set & laws::idempotent) != 0; }

// A nonempty associative quasigroup has an identity element.
constexpr bool associative_divisible_implies_unital(law_set set) noexcept
{
  return (set & associative_divisible) != associative_divisible || (set & laws::unital) != 0;
}

}  // namespace detail

constexpr bool is_valid_law_set(law_set set) noexcept
{
  return detail::uses_only_known_laws(set) && detail::selective_implies_idempotent(set) && detail::associative_divisible_implies_unital(set);
}

namespace detail {

template<law_set Laws>
struct checked_structure_tag {
  static_assert(uses_only_known_laws(Laws), "law_set uses a reserved bit");
  static_assert(selective_implies_idempotent(Laws), "selective implies idempotent");
  static_assert(associative_divisible_implies_unital(Laws), "associative and divisible imply unital");
  using type = law_lattice<Laws> const*;
};

}  // namespace detail

// A pointer rather than the class itself, so tags are trivially copyable and
// usable in constant expressions despite the virtual bases.
//
// Checking the law set here makes Laws non-deducible from structure_tag<Laws>.
// That is deliberate: a deducing overload would match every tag exactly and so
// outrank every concrete one. Use structure_tag<structures::magma> as the
// fallback overload instead.
template<law_set Laws>
using structure_tag = typename detail::checked_structure_tag<Laws>::type;

template<law_set Laws>
constexpr structure_tag<Laws> make_structure_tag() noexcept
{
  return nullptr;
}

namespace structures {

constexpr law_set magma = law_set{0};

// clang-format off
constexpr law_set commutative_magma      = magma                                                      | laws::commutative                    ;
constexpr law_set unital_magma           = magma                     | laws::unital                                                          ;
constexpr law_set semigroup              = magma | laws::associative                                                                         ;
constexpr law_set commutative_semigroup  = magma | laws::associative                                  | laws::commutative                    ;
constexpr law_set band                   = magma | laws::associative                                                      | laws::idempotent ;
constexpr law_set semilattice            = magma | laws::associative                                  | laws::commutative | laws::idempotent ;
constexpr law_set monoid                 = magma | laws::associative | laws::unital                                                          ;
constexpr law_set commutative_monoid     = magma | laws::associative | laws::unital                   | laws::commutative                    ;
constexpr law_set idempotent_monoid      = magma | laws::associative | laws::unital                                       | laws::idempotent ;
constexpr law_set bounded_semilattice    = magma | laws::associative | laws::unital                   | laws::commutative | laws::idempotent ;
constexpr law_set quasigroup             = magma                                    | laws::divisible                                        ;
constexpr law_set commutative_quasigroup = magma                                    | laws::divisible | laws::commutative                    ;
constexpr law_set idempotent_quasigroup  = magma                                    | laws::divisible                     | laws::idempotent ;
constexpr law_set loop                   = magma                     | laws::unital | laws::divisible                                        ;
constexpr law_set commutative_loop       = magma                     | laws::unital | laws::divisible | laws::commutative                    ;
constexpr law_set group                  = magma | laws::associative | laws::unital | laws::divisible                                        ;
constexpr law_set commutative_group      = magma | laws::associative | laws::unital | laws::divisible | laws::commutative                    ;
// clang-format on

}  // namespace structures

}  // namespace cats

}  // namespace yk

#endif  // YK_ZZ_CATS_HPP
