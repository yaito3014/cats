#ifndef YK_ZZ_CATS_HPP
#define YK_ZZ_CATS_HPP

#include <functional>
#include <type_traits>
#include <utility>

#include <cassert>
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

// An instance set selects which law declarations are in effect, so that the
// same operation can be declared differently for different needs. A set may
// name a fallback, whose declarations it inherits and may only strengthen.
namespace instances {

// Laws that hold exactly, such as those of unsigned arithmetic.
struct strict {};

// Additionally, laws that hold only up to overflow or rounding, such as those
// of signed and floating-point arithmetic.
struct lenient {
  using fallback = strict;
};

}  // namespace instances

// Declares the laws of Op on the carrier T under Instances, by deriving from
// std::integral_constant<law_set, Laws>. Specializations declare laws; the
// primary template provides those of the standard operations and otherwise
// inherits from the fallback of Instances, if any.
template<class Op, class T, class Instances>
struct operation_laws;

namespace detail {

template<class T>
struct make_void {
  using type = void;
};

template<class Laws, class = void>
struct declares_laws : std::false_type {};

template<class Laws>
struct declares_laws<Laws, typename make_void<decltype(Laws::value)>::type> : std::true_type {};

enum class builtin_operation { none, plus, multiplies, bit_and, bit_or, bit_xor };

template<class Op>
struct builtin_operation_of : std::integral_constant<builtin_operation, builtin_operation::none> {};

template<class U>
struct builtin_operation_of<std::plus<U>> : std::integral_constant<builtin_operation, builtin_operation::plus> {
  using operand_type = U;
};

template<class U>
struct builtin_operation_of<std::multiplies<U>> : std::integral_constant<builtin_operation, builtin_operation::multiplies> {
  using operand_type = U;
};

template<class U>
struct builtin_operation_of<std::bit_and<U>> : std::integral_constant<builtin_operation, builtin_operation::bit_and> {
  using operand_type = U;
};

template<class U>
struct builtin_operation_of<std::bit_or<U>> : std::integral_constant<builtin_operation, builtin_operation::bit_or> {
  using operand_type = U;
};

template<class U>
struct builtin_operation_of<std::bit_xor<U>> : std::integral_constant<builtin_operation, builtin_operation::bit_xor> {
  using operand_type = U;
};

// Integral promotion leaves T unchanged, so a transparent operator applied to
// two Ts yields a T and unsigned arithmetic on T is performed modulo 2^N.
template<class T, bool = std::is_arithmetic<T>::value>
struct is_unpromoted_arithmetic : std::false_type {};

template<class T>
struct is_unpromoted_arithmetic<T, true> : std::is_same<decltype(+std::declval<T>()), T> {};

// The standard operation Op is closed on T: it is either Op<T>, or the
// transparent Op<> and T is not promoted.
template<class Op, class T, class Operand = typename builtin_operation_of<Op>::operand_type>
struct is_closed_builtin_operation : std::is_same<Operand, T> {};

template<class Op, class T>
struct is_closed_builtin_operation<Op, T, void> : is_unpromoted_arithmetic<T> {};

template<class Op, class T, bool = builtin_operation_of<Op>::value != builtin_operation::none>
struct builtin_operation_on : std::integral_constant<builtin_operation, builtin_operation::none> {};

template<class Op, class T>
struct builtin_operation_on<Op, T, true>
    : std::integral_constant<builtin_operation, is_closed_builtin_operation<Op, T>::value ? builtin_operation_of<Op>::value : builtin_operation::none> {};

constexpr bool is_bitwise(builtin_operation op) noexcept
{
  return op == builtin_operation::bit_and || op == builtin_operation::bit_or || op == builtin_operation::bit_xor;
}

template<class T>
constexpr bool has_exact_builtin_laws(builtin_operation op) noexcept
{
  return op != builtin_operation::none
         && ((std::is_integral<T>::value && std::is_unsigned<T>::value && is_unpromoted_arithmetic<T>::value)
             || (is_bitwise(op) && std::is_same<T, bool>::value));
}

template<class T>
constexpr bool has_builtin_laws(builtin_operation op) noexcept
{
  return op != builtin_operation::none && (is_bitwise(op) ? std::is_integral<T>::value : std::is_arithmetic<T>::value && !std::is_same<T, bool>::value);
}

constexpr law_set builtin_laws(builtin_operation op) noexcept
{
  return op == builtin_operation::plus || op == builtin_operation::bit_xor ? structures::commutative_group
         : op == builtin_operation::multiplies                             ? structures::commutative_monoid
                                                                           : structures::bounded_semilattice;
}

template<class Op, class T, class Instances>
struct builtin_operation_laws {};

template<class Op, class T, bool = has_exact_builtin_laws<T>(builtin_operation_on<Op, T>::value)>
struct exact_builtin_operation_laws {};

template<class Op, class T>
struct exact_builtin_operation_laws<Op, T, true> : std::integral_constant<law_set, builtin_laws(builtin_operation_on<Op, T>::value)> {};

template<class Op, class T>
struct builtin_operation_laws<Op, T, instances::strict> : exact_builtin_operation_laws<Op, T> {};

template<class Op, class T, bool = !has_exact_builtin_laws<T>(builtin_operation_on<Op, T>::value) && has_builtin_laws<T>(builtin_operation_on<Op, T>::value)>
struct inexact_builtin_operation_laws {};

template<class Op, class T>
struct inexact_builtin_operation_laws<Op, T, true> : std::integral_constant<law_set, builtin_laws(builtin_operation_on<Op, T>::value)> {};

template<class Op, class T>
struct builtin_operation_laws<Op, T, instances::lenient> : inexact_builtin_operation_laws<Op, T> {};

template<class Op, class T, class Instances, class = void>
struct fallback_operation_laws {};

template<class Op, class T, class Instances>
struct fallback_operation_laws<Op, T, Instances, typename make_void<typename Instances::fallback>::type> : operation_laws<Op, T, typename Instances::fallback> {
};

template<class Op, class T, class Instances>
struct default_operation_laws : std::conditional<
                                    declares_laws<builtin_operation_laws<Op, T, Instances>>::value, builtin_operation_laws<Op, T, Instances>,
                                    fallback_operation_laws<Op, T, Instances>>::type {};

}  // namespace detail

template<class Op, class T, class Instances>
struct operation_laws : detail::default_operation_laws<Op, T, Instances> {};

namespace detail {

template<class Laws, bool = declares_laws<Laws>::value>
struct declared_laws : std::integral_constant<law_set, structures::magma> {};

template<class Laws>
struct declared_laws<Laws, true> : std::integral_constant<law_set, Laws::value> {};

// Every instance set in the chain from Instances declares at least the laws of
// its fallback.
template<class Op, class T, class Instances, class = void>
struct strengthens_fallbacks : std::true_type {};

template<class Op, class T, class Instances>
struct strengthens_fallbacks<Op, T, Instances, typename make_void<typename Instances::fallback>::type>
    : std::integral_constant<
          bool,
          (!declares_laws<operation_laws<Op, T, typename Instances::fallback>>::value
           || (declares_laws<operation_laws<Op, T, Instances>>::value
               && (declared_laws<operation_laws<Op, T, typename Instances::fallback>>::value & ~declared_laws<operation_laws<Op, T, Instances>>::value) == 0))
              && strengthens_fallbacks<Op, T, typename Instances::fallback>::value> {};

template<class Op, class T, class Instances>
struct checked_operation_laws {
  static_assert(declares_laws<operation_laws<Op, T, Instances>>::value, "no laws are declared for the operation on the carrier");
  static_assert(strengthens_fallbacks<Op, T, Instances>::value, "an instance set must not weaken the laws of its fallback");
  static constexpr law_set value = declared_laws<operation_laws<Op, T, Instances>>::value;
};

}  // namespace detail

template<class Op, class T, class Instances = instances::lenient>
using operation_structure_tag = structure_tag<detail::checked_operation_laws<Op, T, Instances>::value>;

// Query keys. An operation answers identity_element<T>(op) through a const
// member op.query(identity_element_t<T>{}), and inverse(op, x) through
// op.query(inverse_t{}, x). A member query takes precedence over the traits
// below, and can depend on the state of the operation.
template<class T>
struct identity_element_t {};

struct inverse_t {};

// Specializations provide
//   static constexpr T identity_element(Op const&);
// returning the identity element of Op on T. The primary template provides
// those of the standard operations.
template<class Op, class T>
struct identity_element_traits;

// Specializations provide
//   static constexpr T inverse(Op const&, T const&);
// returning the inverse of an element under Op. The primary template provides
// those of the standard operations.
template<class Op, class T>
struct inverse_traits;

namespace detail {

template<builtin_operation Op, class T>
struct builtin_identity_element {};

template<class T>
struct builtin_identity_element<builtin_operation::plus, T> {
  template<class Op>
  static constexpr T identity_element(Op const&) noexcept
  {
    return T(0);
  }
};

template<class T>
struct builtin_identity_element<builtin_operation::multiplies, T> {
  template<class Op>
  static constexpr T identity_element(Op const&) noexcept
  {
    return T(1);
  }
};

template<class T>
struct builtin_identity_element<builtin_operation::bit_and, T> {
  template<class Op>
  static constexpr T identity_element(Op const&) noexcept
  {
    return static_cast<T>(-1);
  }
};

template<class T>
struct builtin_identity_element<builtin_operation::bit_or, T> : builtin_identity_element<builtin_operation::plus, T> {};

template<class T>
struct builtin_identity_element<builtin_operation::bit_xor, T> : builtin_identity_element<builtin_operation::plus, T> {};

template<builtin_operation Op, class T>
struct builtin_inverse {};

template<class T>
struct builtin_inverse<builtin_operation::plus, T> {
  template<class Op>
  static constexpr T inverse(Op const&, T const& x) noexcept
  {
    return static_cast<T>(-x);
  }
};

template<class T>
struct builtin_inverse<builtin_operation::bit_xor, T> {
  template<class Op>
  static constexpr T inverse(Op const&, T const& x) noexcept
  {
    return x;
  }
};

template<class Op, class T>
struct builtin_operation_with_laws
    : std::integral_constant<
          builtin_operation, has_builtin_laws<T>(builtin_operation_on<Op, T>::value) ? builtin_operation_on<Op, T>::value : builtin_operation::none> {};

template<class Op, class T, class = void>
struct has_identity_element_query : std::false_type {};

template<class Op, class T>
struct has_identity_element_query<Op, T, typename make_void<decltype(std::declval<Op const&>().query(identity_element_t<T>()))>::type> : std::true_type {};

template<class Op, class T, class = void>
struct has_identity_element_traits : std::false_type {};

template<class Op, class T>
struct has_identity_element_traits<Op, T, typename make_void<decltype(identity_element_traits<Op, T>::identity_element(std::declval<Op const&>()))>::type>
    : std::true_type {};

template<class Op, class T, class = void>
struct has_inverse_query : std::false_type {};

template<class Op, class T>
struct has_inverse_query<Op, T, typename make_void<decltype(std::declval<Op const&>().query(inverse_t(), std::declval<T const&>()))>::type> : std::true_type {};

template<class Op, class T, class = void>
struct has_inverse_traits : std::false_type {};

template<class Op, class T>
struct has_inverse_traits<Op, T, typename make_void<decltype(inverse_traits<Op, T>::inverse(std::declval<Op const&>(), std::declval<T const&>()))>::type>
    : std::true_type {};

}  // namespace detail

template<class Op, class T>
struct identity_element_traits : detail::builtin_identity_element<detail::builtin_operation_with_laws<Op, T>::value, T> {};

template<class Op, class T>
struct inverse_traits : detail::builtin_inverse<detail::builtin_operation_with_laws<Op, T>::value, T> {};

namespace detail {

template<class T, class Op>
constexpr T query_identity_element(Op const& op, std::true_type /* has_identity_element_query */)
{
  return op.query(identity_element_t<T>());
}

template<class T, class Op>
constexpr T query_identity_element(Op const& op, std::false_type /* has_identity_element_query */)
{
  return identity_element_traits<Op, T>::identity_element(op);
}

template<class Op, class T>
constexpr T query_inverse(Op const& op, T const& x, std::true_type /* has_inverse_query */)
{
  return op.query(inverse_t(), x);
}

template<class Op, class T>
constexpr T query_inverse(Op const& op, T const& x, std::false_type /* has_inverse_query */)
{
  return inverse_traits<Op, T>::inverse(op, x);
}

template<class Op>
constexpr Op make_stateless_operation()
{
  static_assert(std::is_default_constructible<Op>::value, "an overload without an operation parameter requires a default-constructible operation");
  return Op();
}

}  // namespace detail

template<class T, class Op>
constexpr T identity_element(Op const& op)
{
  static_assert(
      detail::has_identity_element_query<Op, T>::value || detail::has_identity_element_traits<Op, T>::value,
      "no identity element is declared for the operation on the carrier"
  );
  return detail::query_identity_element<T>(op, detail::has_identity_element_query<Op, T>());
}

template<class T, class Op>
constexpr T identity_element()
{
  return identity_element<T>(detail::make_stateless_operation<Op>());
}

template<class Op, class T>
constexpr T inverse(Op const& op, T const& x)
{
  static_assert(detail::has_inverse_query<Op, T>::value || detail::has_inverse_traits<Op, T>::value, "no inverse is declared for the operation on the carrier");
  return detail::query_inverse(op, x, detail::has_inverse_query<Op, T>());
}

template<class Op, class T>
constexpr T inverse(T const& x)
{
  return inverse(detail::make_stateless_operation<Op>(), x);
}

namespace detail {

template<class Integer>
constexpr bool is_negative(Integer n, std::true_type /* is_signed */) noexcept
{
  return n < 0;
}

template<class Integer>
constexpr bool is_negative(Integer, std::false_type /* is_signed */) noexcept
{
  return false;
}

template<class Integer>
constexpr bool is_negative(Integer n) noexcept
{
  return is_negative(n, std::is_signed<Integer>());
}

// |n|, computed in the unsigned type so that it is exact for the minimum value.
template<class Integer>
constexpr typename std::make_unsigned<Integer>::type magnitude(Integer n) noexcept
{
  using unsigned_type = typename std::make_unsigned<Integer>::type;
  return is_negative(n) ? static_cast<unsigned_type>(unsigned_type(0) - static_cast<unsigned_type>(n)) : static_cast<unsigned_type>(n);
}

// x^n for n >= 1 by repeated squaring, which needs only associativity.
template<class Op, class T, class Unsigned>
constexpr T power_positive(Op const& op, T const& x, Unsigned n)
{
  return n == 1       ? x
         : n % 2 == 0 ? power_positive(op, T(op(x, x)), static_cast<Unsigned>(n / 2))
                      : T(op(x, power_positive(op, T(op(x, x)), static_cast<Unsigned>(n / 2))));
}

template<class Op, class T, class Integer>
constexpr T power_impl(structure_tag<structures::semigroup>, Op const& op, T const& x, Integer n)
{
  return assert(n >= 1), power_positive(op, x, magnitude(n));
}

template<class Op, class T, class Integer>
constexpr T power_impl(structure_tag<structures::monoid>, Op const& op, T const& x, Integer n)
{
  return assert(!is_negative(n)), n == 0 ? identity_element<T>(op) : power_positive(op, x, magnitude(n));
}

template<class Op, class T, class Integer>
constexpr T power_impl(structure_tag<structures::group>, Op const& op, T const& x, Integer n)
{
  return n == 0 ? identity_element<T>(op) : power_positive(op, is_negative(n) ? inverse(op, x) : x, magnitude(n));
}

template<class Op, class T, class Integer>
constexpr T power_impl(structure_tag<structures::band>, Op const&, T const& x, Integer n)
{
  return assert(n >= 1), x;
}

template<class Op, class T, class Integer>
constexpr T power_impl(structure_tag<structures::idempotent_monoid>, Op const& op, T const& x, Integer n)
{
  return assert(!is_negative(n)), n == 0 ? identity_element<T>(op) : x;
}

// Only a one-element carrier satisfies these laws, so every power is x.
template<class Op, class T, class Integer>
constexpr T power_impl(structure_tag<structures::group | laws::idempotent>, Op const&, T const& x, Integer)
{
  return x;
}

}  // namespace detail

// x combined with itself n times. Requires n >= 1, or n >= 0 for a unital
// operation; a group also accepts negative n.
template<class Instances = instances::lenient, class Op, class T, class Integer>
constexpr T power(Op const& op, T const& x, Integer n)
{
  static_assert(std::is_integral<Integer>::value && !std::is_same<Integer, bool>::value, "power requires an integer exponent");
  static_assert((detail::checked_operation_laws<Op, T, Instances>::value & laws::associative) != 0, "power requires an associative operation");
  return detail::power_impl(operation_structure_tag<Op, T, Instances>(), op, x, n);
}

template<class Op, class Instances = instances::lenient, class T, class Integer>
constexpr T power(T const& x, Integer n)
{
  return power<Instances>(detail::make_stateless_operation<Op>(), x, n);
}

}  // namespace cats

}  // namespace yk

#endif  // YK_ZZ_CATS_HPP
