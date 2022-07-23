/**
 * @file units.hpp
 * @author Stephen Verderame (stephenverderame@gmail.com)
 * @brief This file provides a unit type that allows semantic typing of arithmetic
 *       operations on units. The Unit class provides compile-time 
 *       dimensional analysis in order to check some aspects of computational 
 *       correctness. Using the unit class, the compiler will be able to ensure
 *       you cannot accidentally compute an area where a volume is expected,
 *       you cannot accidentally pass a width where a height was expected,
 *       ensures correct scaling of units, and so on.
 * 
 *       The Unit class has semantic types (such as Length, Width, Depth, etc.) that
 *       can be used to differentiate values of the same SI unit, and real units
 *       (such as meters, seconds, etc.)
 * 
 *       A Unit can opt not to use any SemanticType by using NoSemanticType. A
 *       semantic type and real unit type can be an struct or class that derives from
 *       UnitBase. For ease of use, you can use the SEMANTIC_UNIT_TYPE macro as the
 *       base class for your semantic types.
 * 
 *      std=c++17
 *       
 * @version 0.1
 * @date 2022-07-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <type_traits>
#include <typeinfo>
#include <cstdint>
#include <cmath>
using std::size_t;

template<size_t N>
constexpr size_t file_hash(char const (&)[N])
{
  return N-1;
}

/// The base class for all unit types
/// Gives the inheriter a unique constexpr id
template<uint32_t major, uint32_t minor>
struct UnitBase {
    static constexpr uint64_t id = uint64_t(major) << 32 | minor;
};

#define STRFY(X) #X
#define STRFY2(X) STRFY(X)
/// The base class for all unit types
#define SEMANTIC_UNIT_TYPE UnitBase<file_hash(__FILE__), __LINE__>

template<bool expr, typename TrueType, typename FalseType>
struct TypeConditional {
    using Type = FalseType;
};

template<typename TrueType, typename FalseType>
struct TypeConditional<true, TrueType, FalseType> {
    using Type = TrueType;
};


template<bool expr, typename TrueType, typename FalseType>
/// TrueType if expr is true, FalseType otherwise
using type_cond_t = typename TypeConditional<expr, TrueType, FalseType>::Type;

template<typename A, typename B>
struct UnitIdComparator {
    static constexpr bool value = A::id < B::id;
};

/// Gets the minimum type in the parameter pack as denotes by the comparator
/// @tparam Comparator a type with a `value` member that is `true` if its first type
///     parameter is less than its second type, `false` otherwise.
//      Comparator should establish a partial ordering
/// @tparam Head head of the parameter pack
/// @tparam Tail tail of the parameter pack
template<template <typename, typename> class Comparator, typename Head, typename ... Tail>
struct MinType {
private:
    using TailMinType = typename MinType<Comparator, Tail...>::Type;
public:
    using Type = type_cond_t<Comparator<Head, TailMinType>::value, Head, TailMinType>;
};

template<template <typename, typename> class Comparator, typename Last>
struct MinType<Comparator, Last> {
    using Type = Last;
};

template<typename ... Ts>
/// The minimum of all the unit types in the template list
/// Requires all types to have an id member, which is used to order them
using min_unit_type_t = typename MinType<UnitIdComparator, Ts...>::Type;

/// A type list
/// This type list is represented just as a parameter pack
/// The type list does not store a tail node, but operations on the type list
/// will return `Pack<EmptyPack>` when there are no more elements
template<typename ... Ts>
struct Pack {};

/// Denotes an empty type list `Pack<EmptyPack>`
struct EmptyPack {
    static constexpr uint64_t id = ~0;
};

template<typename ... Ts>
/// The return type is the minimum of all the types in the pack
constexpr auto pack_min(Pack<Ts...> p) {
    return min_unit_type_t<Ts...>{};
}

/// The return type is a Pack of T consed onto Ts
/// Consing an element of type T onto a Pack<EmptyPack> will result in Pack<T>
/// @{
template<typename T, typename ... Ts>
constexpr auto pack_cons(T a, Pack<Ts...> p) {
    return Pack<T, Ts...>{};
}

template<typename T>
constexpr auto pack_cons(T a, Pack<EmptyPack> p) {
    return Pack<T>{};
}
/// @}

template<typename T, typename Pack>
/// Gets the Pack of T consed onto Pack
/// Consing an element of type T onto a Pack<EmptyPack> will result in Pack<T>
using pack_cons_t = decltype(pack_cons(std::declval<T>(), std::declval<Pack>()));

/// Removes the first occurrence of T from the list
/// Returns a Pack of types without T
/// @{
template<typename T, typename Head, typename ... Tail>
struct RemoveType {
    using Type = type_cond_t<std::is_same_v<T, Head>, Pack<Tail...>, 
        pack_cons_t<Head, typename RemoveType<T, Tail...>::Type>>;
};

template<typename T, typename Head>
struct RemoveType<T, Head> {
    using Type = type_cond_t<std::is_same_v<T, Head>, Pack<EmptyPack>, Pack<Head>>;
};
/// @}

/// Removes the first occurrence of T from the the Pack
/// @{
template<typename T, typename ... Ts>
constexpr auto remove_from_pack(T a, Pack<Ts...> p) {
    return typename RemoveType<T, Ts...>::Type{};
}

template<typename T, typename Pack>
using remove_pack_t = decltype(remove_from_pack(std::declval<T>(), std::declval<Pack>()));
/// @}

/// Returns a sorted pack of types, ordered by the comparator via Selection Sort
/// @tparam Comparator - a type with a `value` member that is `true` if its first type
///     parameter should be ordered before its second
/// @tparam ... Ts - parameter pack of types to sort
/// @returns a PACK of the ordered types. So this type constructs a sorted type list
///     from a parameter pack
/// @{
template<template<typename, typename> class Comparator, typename ... Ts>
struct SortTypes;

template<template<typename, typename> class Comparator, typename ... Ts>
constexpr auto sort_pack(Pack<Ts...> p) {
    return typename SortTypes<Comparator, Ts...>::Type{};
}

template<template<typename, typename> class Comparator, typename Pack>
using sort_pack_t = decltype(sort_pack<Comparator>(std::declval<Pack>()));

template<typename Pack>
using sort_unit_pack_t = sort_pack_t<UnitIdComparator, Pack>;

template<template<typename, typename> class Comparator, typename ... List>
struct SortTypes {
private:
    using Min = typename MinType<Comparator, List...>::Type;
    using TailPack = typename RemoveType<Min, List...>::Type;
public:
    using Type = pack_cons_t<Min, sort_pack_t<Comparator, TailPack>>;
};

template<template<typename, typename> class Comparator, typename Last>
struct SortTypes<Comparator, Last> {
    using Type = Pack<Last>;
};

/// @}

/// Gets the head of a type list. Returns `Pack<EmptyPack>` if the list is empty
template<typename Head, typename ... Tail>
constexpr auto pack_head(Pack<Head, Tail...> p) {
    return Head{};
}

constexpr auto pack_head(Pack<EmptyPack> p) {
    return EmptyPack{};
}

/// Gets the tail of the type list. Returns `Pack<EmptyPack>` if there is no tail
template<typename Head, typename ... Tail>
constexpr auto pack_tail(Pack<Head, Tail...> p, float)
    -> std::enable_if_t<sizeof...(Tail) != 0, Pack<Tail...>> 
{
    return Pack<Tail...>{};
}

template<typename T>
constexpr auto pack_tail(Pack<T> p, int) {
    return Pack<EmptyPack>{};
}


template<typename Pack>
using pack_head_t = decltype(pack_head(std::declval<Pack>()));

template<typename Pack>
using pack_tail_t = decltype(pack_tail(std::declval<Pack>(), 0));

/// Returns a pack of types of the types in PackA followed by the types in PackB
/// @{
template<typename PackA, typename PackB>
struct PackConcat;

template<typename PackB>
struct PackConcat<Pack<EmptyPack>, PackB> {
    using Type = PackB;
};

template<typename PackA>
struct PackConcat<PackA, Pack<EmptyPack>> {
    using Type = PackA;
};

template<>
struct PackConcat<Pack<EmptyPack>, Pack<EmptyPack>> {
    using Type = Pack<EmptyPack>;
};

template<typename PackA, typename PackB>
struct PackConcat {
    using Type = pack_cons_t<pack_head_t<PackA>, typename PackConcat<pack_tail_t<PackA>, PackB>::Type>;
};

template<typename PackA, typename PackB>
using pack_concat_t = typename PackConcat<PackA, PackB>::Type;
/// @}

/// Removes all of PackA from PackB
/// @{
template<typename PackA, typename PackB>
struct PackRemoveAll;

template<typename PackB>
struct PackRemoveAll<Pack<EmptyPack>, PackB> {
    using Type = PackB;
};

template<typename PackA>
struct PackRemoveAll<PackA, Pack<EmptyPack>> {
    using Type = Pack<EmptyPack>;
};

template<typename PackA, typename PackB>
struct PackRemoveAll {
    using Type = remove_pack_t<pack_head_t<PackA>, 
        typename PackRemoveAll<pack_tail_t<PackA>, PackB>::Type>;
};

/// Removes all of PackA from PackB
template<typename PackA, typename PackB>
using pack_remove_all_t = typename PackRemoveAll<PackA, PackB>::Type;
/// @}

constexpr int gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

/// A unit raised to a rational power
template<typename Unit, int32_t num, int32_t den>
struct PowerType {
    constexpr static uint64_t id = Unit::id;
    constexpr static double power = static_cast<double>(num) / den;

    static_assert(den > 0, "Power type denominator must be > 0");
    static_assert(gcd(den, num) == 1 || gcd(den, num) == -1, 
        "Power type numerator and denominator must be in lowest terms");
};

template<typename Unit, int32_t numA, int32_t denA, int32_t numB, int32_t denB>
constexpr auto power_add(PowerType<Unit, numA, denA>, PowerType<Unit, numB, denB>) {
    constexpr auto num = numA * denB + numB * denA;
    constexpr auto den = denA * denB;
    constexpr auto div = gcd(num, den);
    constexpr auto den_ = den / div < 0 ? -den / div : den / div;
    constexpr auto num_ = den / div < 0 ? -num / div : num / div;
    return PowerType<Unit, num_, den_>{};
}

// This overload is needed for type checking, but should never get called as two 
// types of different base units should not be added together
template<typename A, typename B>
constexpr A power_add(A a, B) {
    throw std::invalid_argument("Cannot add two units of different base units");
    return a;
};

/// Combines the powers of two units by adding them
/// Requires the units share the same base unit
template<typename UnitA, typename UnitB>
using add_unit_powers_t = decltype(
    power_add(std::declval<UnitA>(), std::declval<UnitB>()));

/// Multiplies two powers together
template<typename Unit, int32_t numA, int32_t denA, int32_t numB, int32_t denB>
constexpr auto power_mult(PowerType<Unit, numA, denA>, PowerType<Unit, numB, denB>) {
    constexpr auto num = numA * numB;
    constexpr auto den = denA * denB;
    constexpr auto div = gcd(num, den);
    constexpr auto den_ = den / div < 0 ? -den / div : den / div;
    constexpr auto num_ = den / div < 0 ? -num / div : num / div;
    return PowerType<Unit, num_, den_>{};
}

// This overload is needed for type checking, but should never get called as two types of
// different base units should not be added together
template<typename A, typename B>
constexpr A power_mult(A a, B) {
    throw std::invalid_argument("Cannot add two units of different base units");
    return a;
};

/// Combines the powers of two units by multiplying them
/// Requires the units share the same base unit
template<typename UnitA, typename UnitB>
using mult_unit_powers_t = decltype(
    power_mult(std::declval<UnitA>(), std::declval<UnitB>()));

template<typename A, typename B>
struct same_power_unit : std::false_type {};

template<typename Unit, int32_t numA, int32_t numB, int32_t denA, int32_t denB>
struct same_power_unit<PowerType<Unit, numA, denA>, PowerType<Unit, numB, denB>> 
    : std::true_type {};

/// True if two power types have the same base unit. Their powers may not match
template<typename UnitA, typename UnitB>
constexpr bool same_unit_v = same_power_unit<UnitA, UnitB>::value;

/// Adds the PowerType to the end of the PowerPack unless the PowerPack contains 
/// a PowerType with the same base unit
/// if so, then combines the exponents of both units via the PowerCombiner
/// @tparam PowerType power type to add to the pack
/// @tparam PowerPack existing pack of power types
/// @tparam PowerCombiner operation on exponents to combine powers of the same base unit
template<typename PowerType, typename PowerPack, template <typename, typename> class PowerCombiner>
struct ConsPowerType {
private:
    using Head = pack_head_t<PowerPack>;
    using Tail = pack_tail_t<PowerPack>;
public:
    using Type = type_cond_t<same_unit_v<PowerType, Head>,
        pack_cons_t<PowerCombiner<PowerType, Head>, Tail>,
        pack_cons_t<Head, typename ConsPowerType<PowerType, Tail, PowerCombiner>::Type>>;
};

template<typename PowerType, template <typename, typename> class PowerCombiner>
struct ConsPowerType<PowerType, Pack<EmptyPack>, PowerCombiner> {
    using Type = Pack<PowerType>;
};

/// Adds a power type to a power pack, combining it by adding exponents
/// if the unit is already present in the pack
/// If no unit is present, adds the new power type to the END of the pack
template<typename PowerType, typename PowerPack>
using cons_power_add_t = typename ConsPowerType<PowerType, PowerPack, add_unit_powers_t>::Type;

/// Adds a power type to a power pack, combining it by multiplying exponents
/// if the unit is already present in the pack
/// If no unit is present, adds the new power type to the END of the pack
template<typename PowerType, typename PowerPack>
using cons_power_mult_t = typename ConsPowerType<PowerType, PowerPack, mult_unit_powers_t>::Type;

template<typename PowerType>
struct NegatePowerType {};

template<typename Unit, int32_t num, int32_t den>
struct NegatePowerType<PowerType<Unit, num, den>> {
    using Type = PowerType<Unit, -num, den>;
};

/// Negates the power that a unit is raised to
template<typename PowerType>
using negate_power_t = typename NegatePowerType<PowerType>::Type;

template<typename PowerPack>
struct NegatePowerPack {
    using Type = pack_cons_t<negate_power_t<pack_head_t<PowerPack>>, 
        typename NegatePowerPack<pack_tail_t<PowerPack>>::Type>;
};

template<>
struct NegatePowerPack<Pack<EmptyPack>> {
    using Type = Pack<EmptyPack>;
};

/// Negates all the powers of the units in the power pack
template<typename PowerPack>
using negate_power_pack_t = typename NegatePowerPack<PowerPack>::Type;

template<typename PowerType>
struct IsZeroPowerType : std::false_type {};

template<typename Unit, int32_t den>
struct IsZeroPowerType<PowerType<Unit, 0, den>> : std::true_type {};

/// Determines if a power type is raided to the zeroth power zero
template<typename PowerType>
constexpr bool is_zero_power_v = IsZeroPowerType<PowerType>::value;

/// Removes any power types in the pack with a zero exponent
/// @{
template<typename PowerPack>
struct CleanPowerPack {
private:
    using Head = pack_head_t<PowerPack>;
    using Tail = pack_tail_t<PowerPack>;
public:
    using Type = type_cond_t<is_zero_power_v<Head>,
        typename CleanPowerPack<Tail>::Type,
        pack_cons_t<Head, typename CleanPowerPack<Tail>::Type>>;
};

template<>
struct CleanPowerPack<Pack<EmptyPack>> {
    using Type = Pack<EmptyPack>;
};
/// @}

/// Removes all zero power units from the pack
template<typename PowerPack>
using clean_power_pack_t = typename CleanPowerPack<PowerPack>::Type;

/// Returns a pack of power types of the types in PackA followed by the types in PackB
/// Combines the powers of the units that are the same in PackA and PackB
/// @tparam PackA first pack of power types
/// @tparam PackB second pack of power types
/// @tparam PowerCombiner operation on exponents to cons the powers of the same base unit
/// @{
template<typename PackA, typename PackB, template<typename, typename> class PowerCombiner>
struct PowerPackCombine;

template<typename PackB, template<typename, typename> class PowerCombiner>
struct PowerPackCombine<Pack<EmptyPack>, PackB, PowerCombiner> {
    using Type = PackB;
};

template<typename PackA, template<typename, typename> class PowerCombiner>
struct PowerPackCombine<PackA, Pack<EmptyPack>, PowerCombiner> {
    using Type = PackA;
};

template<template<typename, typename> class PowerCombiner>
struct PowerPackCombine<Pack<EmptyPack>, Pack<EmptyPack>, PowerCombiner> {
    using Type = Pack<EmptyPack>;
};

/// Merges the power packs, combining exponents if the units are the same
/// @tparam PackA first pack of power types
/// @tparam PackB second pack of power types
/// @tparam PowerCombiner operation to cons a power type onto a power pack
template<typename PackA, typename PackB, template<typename, typename> class PowerCombiner>
struct PowerPackCombine {
    using Type = PowerCombiner<pack_head_t<PackA>, 
        typename PowerPackCombine<pack_tail_t<PackA>, PackB, PowerCombiner>::Type>;
};

/// Combines the power packs, merging any units that are the same in both packs
/// by adding exponents
template<typename PackA, typename PackB>
using power_pack_add_t = 
    typename PowerPackCombine<PackA, PackB, cons_power_add_t>::Type;

/// Combines the power packs, merging any units that are the same in both packs
/// by multiplying exponents
template<typename PackA, typename PackB>
using power_pack_mult_t = 
    typename PowerPackCombine<PackA, PackB, cons_power_mult_t>::Type;
/// @}

using scale_t = uint64_t;

template<typename Self, typename Other>
constexpr bool is_semantic_convertable_v = std::is_same_v<Self, Other> || 
                                           std::is_same_v<Self, Pack<EmptyPack>> ||
                                           std::is_same_v<Other, Pack<EmptyPack>>;

/// Determines if a pack of units are ordered according to their id
template<typename Pack>
struct IsPackOrdered {
private:
    using Head = pack_head_t<Pack>;
    using Tail = pack_tail_t<Pack>;
    using NextHead = pack_head_t<Tail>;
public:
    static constexpr bool value = Head::id < NextHead::id && 
        IsPackOrdered<Tail>::value;
};

template<>
struct IsPackOrdered<Pack<EmptyPack>> {
    static constexpr bool value = true;
};

/// Determines if a pack of units are ordered according to their id
template<typename Pack>
constexpr bool is_pack_ordered_v = IsPackOrdered<Pack>::value;

template<typename T, scale_t scale, typename SemanticPowerPack, typename UnitPowerPack>
struct Unit;

/// Converts a unit of one semantic subcategory to another
/// @tparam TargetSemantic - the target semantic power pack of the new unit
template<typename TargetSemantic, typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto semantic_cast(const Unit<T, scale, Semantic, Units>& a);

/// Converts a unit of one unit to another
/// @tparam TargetUnits - the target unit power pack of the new unit
template<typename TargetUnits, typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto unit_cast(const Unit<T, scale, Semantic, Units>& a);

/// @brief A semantically typed unit class
/// @tparam T The underlying type of the unit
/// @tparam scale the scale of the unit
/// @tparam Semantic the Pack of subcategory PowerTypes of the unit to 
///     differentiate different values of the same unit. 
///     Ex: `Pack<PowerType<Length, 2, 1>>` which is Length^2
///     Requires every type in the Pack are subtypes of UnitBase
/// @tparam UnitPowerPack the pack of PowerTypes of Units that the value has
///     Ex. `Pack<PowerType<Meters, 2, 1>>`
///     Requires every type in the Pack are subtypes of UnitBase
template<typename T, scale_t scale, typename SemanticPowerPack, typename UnitPowerPack>
struct Unit {
    static_assert(is_pack_ordered_v<SemanticPowerPack> && is_pack_ordered_v<UnitPowerPack>, 
        "SemanticPowerPack and UnitPowerPack must be ordered by id");

    T val;

    explicit constexpr Unit(T v) : val(v) {}

    template<typename OtherSemantic, scale_t otherScale>
    constexpr Unit(const 
        Unit<T, otherScale, OtherSemantic, UnitPowerPack>& other,
        std::enable_if_t<is_semantic_convertable_v<SemanticPowerPack, OtherSemantic>, int> = 0)
        : val(other.val * otherScale / scale) {}


    template<typename OtherSemantic, scale_t otherScale>
    constexpr auto operator=(const Unit<T, otherScale, OtherSemantic, UnitPowerPack>& other)
        -> std::enable_if_t<is_semantic_convertable_v<SemanticPowerPack, OtherSemantic>, Unit&> 
    {
        val = other.val * otherScale / scale;
        return *this;
    }

    constexpr Unit& operator=(T v) {
        val = v;
        return *this;
    }

    constexpr Unit& operator++() {
        ++val;
        return *this;
    }

    constexpr Unit operator++(int) {
        Unit tmp(*this);
        ++(*this);
        return tmp;
    }

    constexpr Unit& operator--() {
        --val;
        return *this;
    }

    constexpr Unit operator--(int) {
        Unit tmp(*this);
        ++(*this);
        return tmp;
    }

    template<typename OtherSemantic, scale_t otherScale>
    constexpr auto operator+=(const Unit<T, otherScale, OtherSemantic, UnitPowerPack>& other) 
        -> std::enable_if_t<is_semantic_convertable_v<SemanticPowerPack, OtherSemantic>, Unit&>
    {
        val += other.val * otherScale / scale;
        return *this;
    }

    template<typename OtherSemantic, scale_t otherScale>
    constexpr auto operator-=(const Unit<T, otherScale, OtherSemantic, UnitPowerPack>& other) 
        -> std::enable_if_t<is_semantic_convertable_v<SemanticPowerPack, OtherSemantic>, Unit&>
    {
        val -= other.val * otherScale / scale;
        return *this;
    }

    constexpr auto operator*=(T v) {
        val *= v;
        return *this;
    }

    constexpr auto operator/=(T v) {
        val /= v;
        return *this;
    }

    /// @brief Returns the value of this unit in the base scale (scale = 1)
    constexpr auto get_in_base_scale() const {
        return val * scale;
    }

    /// Strips the semantic subcategory from the unit
    constexpr auto as_free_unit() const {
        return Unit<T, scale, Pack<EmptyPack>, UnitPowerPack>(val);
    }
};

template<typename T, scale_t scaleA, typename SemanticA, typename UnitPowerPackA,
    scale_t scaleB, typename SemanticB, typename UnitPowerPackB>
constexpr auto operator*(const Unit<T, scaleA, SemanticA, UnitPowerPackA>& a,
    const Unit<T, scaleB, SemanticB, UnitPowerPackB>& b)
{
    using NewPowerPack = clean_power_pack_t<
        sort_unit_pack_t<power_pack_add_t<UnitPowerPackA, UnitPowerPackB>>>;
    using NewSemantic = clean_power_pack_t<
        sort_unit_pack_t<power_pack_add_t<SemanticA, SemanticB>>>;
    if constexpr (std::is_same_v<NewPowerPack, Pack<EmptyPack>>) {
        return a.val * scaleA * b.val * scaleB;
    } else {
        return Unit<T, scaleA * scaleA, NewSemantic, NewPowerPack>
            (a.val * b.val * scaleB / scaleA);
    }
}

template<typename T, scale_t scaleA, typename Semantic, typename UnitPowerPackA,
    scale_t scaleB, typename SemanticB, typename UnitPowerPackB>
constexpr auto operator/(const Unit<T, scaleA, Semantic, UnitPowerPackA>& a,
    const Unit<T, scaleB, SemanticB, UnitPowerPackB>& b)
{
    return a * (T{1} / b);
}

template<typename T, scale_t scale, typename Semantic, typename UnitPowerPack>
constexpr auto operator*(Unit<T, scale, Semantic, UnitPowerPack> a, T b)
{
    a.val *= b;
    return a;
}

template<typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto operator*(T b, Unit<T, scale, Semantic, Units> a)
{
    a.val *= b;
    return a;
}

template<typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto operator/(Unit<T, scale, Semantic, Units> a, T b)
{
    a.val /= b;
    return a;
}

template<typename T, scale_t scale, typename Semantic, typename UnitPowerPack>
constexpr auto operator/(T b, const Unit<T, scale, Semantic, UnitPowerPack>& a)
{
    return Unit<T, scale, negate_power_pack_t<Semantic>, 
        negate_power_pack_t<UnitPowerPack>>(b / a.val);
}

template<typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto operator+(Unit<T, scale, Semantic, Units> a, T b)
{
    a.val += b;
    return a;
}

template<typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto operator+(T b, Unit<T, scale, Semantic, Units> a)
{
    a.val += b;
    return a;
}

template<typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto operator-(Unit<T, scale, Semantic, Units> a, T b)
{
    a.val -= b;
    return a;
}

template<typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto operator-(T b, Unit<T, scale, Semantic, Units> a)
{
    a.val = b - a.val;
    return a;
}

template<typename T, scale_t scale, typename Semantic, typename Units,
    scale_t otherScale, typename OtherSemantic>
constexpr auto operator+(Unit<T, scale, Semantic, Units> a, 
                         const Unit<T, otherScale, OtherSemantic, Units>& b)
    -> std::enable_if_t<is_semantic_convertable_v<Semantic, OtherSemantic>, decltype(a)>
{
    a.val += b.val * otherScale / scale;
    return a;
}

template<typename T, scale_t scale, typename Semantic, typename Units,
    scale_t otherScale, typename OtherSemantic>
constexpr auto operator-(Unit<T, scale, Semantic, Units> a, 
                         const Unit<T, otherScale, OtherSemantic, Units>& b)
    -> std::enable_if_t<is_semantic_convertable_v<Semantic, OtherSemantic>, decltype(a)>
{
    a.val -= b.val * otherScale / scale;
    return a;
}

/// Raises the power of a unit to `powerNum / powerDen`
/// @{
template<typename PowerUnit, int32_t powerNum, int32_t powerDen>
struct RaiseUnitPower {};


template<typename Unit, int32_t unitNum, int32_t unitDen, 
    int32_t powerNum, int32_t powerDen>
struct RaiseUnitPower<PowerType<Unit, unitNum, unitDen>, powerNum, powerDen> {
private:
    constexpr static auto new_num = unitNum * powerNum;
    constexpr static auto new_den = unitDen * powerDen;
    constexpr static auto divisor = gcd(new_num, new_den);
    constexpr static auto den = new_den / divisor < 0 ? 
        -new_den / divisor : new_den / divisor;
    constexpr static auto num = new_den / divisor < 0 ? 
        -new_num / divisor : new_num / divisor;
public:
    using Type = PowerType<Unit, num, den>;
};
/// @}

template<typename PowerPack, int32_t powerNum, int32_t powerDen>
struct RaisePowerPack {
private:
    using Head = pack_head_t<PowerPack>;
    using Tail = pack_tail_t<PowerPack>;
public:
    using Type = pack_cons_t<typename RaiseUnitPower<Head, powerNum, powerDen>::Type,
        typename RaisePowerPack<Tail, powerNum, powerDen>::Type>;

};

template<int32_t powerNum, int32_t powerDen>
struct RaisePowerPack<Pack<EmptyPack>, powerNum, powerDen> {
    using Type = Pack<EmptyPack>;
};


/// Raises all units in a power pack to `powerNum / powerDen`
/// @tparam PowerPack The power pack to raise
/// @tparam powerNum The numerator of the power to raise to
/// @tparam powerDen The denominator of the power to raise to
template<typename PowerPack, int32_t powerNum, int32_t powerDen>
using raise_power_pack_t = typename RaisePowerPack<PowerPack, powerNum, powerDen>::Type;


template<int32_t powerNum, int32_t powerDen, typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto pow(const Unit<T, scale, Semantic, Units>& a)
{
    return Unit<T, 1, raise_power_pack_t<Semantic, powerNum, powerDen>, 
        raise_power_pack_t<Units, powerNum, powerDen>>
            (std::pow(a.val * scale, static_cast<double>(powerNum) / powerDen));
}

template<typename TargetSemantic, typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto semantic_cast(const Unit<T, scale, Semantic, Units>& a)
{
    return Unit<T, scale, TargetSemantic, Units>(a.val);
}

template<typename TargetUnits, typename T, scale_t scale, typename Semantic, typename Units>
constexpr auto unit_cast(const Unit<T, scale, Semantic, Units>& a)
{
    return Unit<T, scale, Semantic, TargetUnits>(a.val);
}

using NoSemanticType = Pack<EmptyPack>;