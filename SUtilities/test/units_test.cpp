#include <iostream>
#include "units.hpp"

struct Meters : SEMANTIC_UNIT_TYPE {};
struct Seconds : SEMANTIC_UNIT_TYPE {};
struct Grams : SEMANTIC_UNIT_TYPE {};

using test_t = sort_unit_pack_t<Pack<Meters, Seconds>>;
using test2_t = sort_unit_pack_t<Pack<Seconds, Meters>>;

struct Length : SEMANTIC_UNIT_TYPE {};
struct Width : SEMANTIC_UNIT_TYPE {};
struct Depth : SEMANTIC_UNIT_TYPE {};

using volume_t = Unit<double, 1, 
    sort_unit_pack_t<Pack<PowerType<Length, 1, 1>, PowerType<Width, 1, 1>, PowerType<Depth, 1, 1>>>,
    Pack<PowerType<Meters, 3, 1>>>;

using area_t = Unit<double, 1, 
    sort_unit_pack_t<Pack<PowerType<Length, 1, 1>, PowerType<Width, 1, 1>>>,
    Pack<PowerType<Meters, 2, 1>>>;

template<typename Semantic>
using dist_t = Unit<double, 1, Pack<PowerType<Semantic, 1, 1>>, Pack<PowerType<Meters, 1, 1>>>;


constexpr auto foo(volume_t vol) {
    constexpr auto val = dist_t<Depth>(7);
    return vol / val;
}

template<typename T>
constexpr char foo(T) { return 'a'; }

constexpr auto bar(area_t area) {
    return area * 2.;
}



int main() {
    static_assert(std::is_same_v<test_t, test2_t>);

    static_assert(std::is_same_v<pack_cons_t<Meters, Pack<Meters, Meters, Seconds, int>>, Pack<Meters, Meters, Meters, Seconds, int>>);
    static_assert(std::is_same_v<pack_cons_t<int, Pack<EmptyPack>>, Pack<int>>);
    static_assert(std::is_same_v<
        sort_unit_pack_t<pack_cons_t<Meters, Pack<Grams, Seconds>>>, 
        sort_unit_pack_t<Pack<Seconds, Meters, Grams>>>);
    static_assert(std::is_same_v<sort_unit_pack_t<Pack<EmptyPack>>, Pack<EmptyPack>>);

    static_assert(std::is_same_v<
        pack_remove_all_t<Pack<char, char, int, long>, Pack<long, char, char, char, double, int>>,
        Pack<char, double>>);

    static_assert(std::is_same_v<add_unit_powers_t<PowerType<Meters, 2, 1>, PowerType<Meters, 3, 1>>, PowerType<Meters, 5, 1>>);
    static_assert(std::is_same_v<add_unit_powers_t<PowerType<Meters, 3, 2>, PowerType<Meters, 7, 3>>, PowerType<Meters, 23, 6>>);
    static_assert(std::is_same_v<add_unit_powers_t<PowerType<Meters, 3, 2>, PowerType<Meters, 7, 2>>, PowerType<Meters, 5, 1>>);

    static_assert(same_unit_v<PowerType<Meters, 2, 1>, PowerType<Meters, 3, 1>>);
    static_assert(!same_unit_v<PowerType<Seconds, 2, 1>, PowerType<Meters, 2, 1>>);

    static_assert(std::is_same_v<
        cons_power_add_t<PowerType<Meters, 2, 1>, Pack<PowerType<Seconds, 1, 1>, PowerType<Meters, 1, 1>>>,
        Pack<PowerType<Seconds, 1, 1>, PowerType<Meters, 3, 1>>>);

    static_assert(std::is_same_v<
        cons_power_add_t<PowerType<Meters, 1, 1>, Pack<EmptyPack>>,
        Pack<PowerType<Meters, 1, 1>>>);

    static_assert(std::is_same_v<
        cons_power_add_t<PowerType<Meters, 1, 1>, Pack<PowerType<Grams, 2, 1>>>,
        Pack<PowerType<Grams, 2, 1>, PowerType<Meters, 1, 1>>>);

    static_assert(std::is_same_v<
        negate_power_pack_t<Pack<PowerType<Meters, 1, 1>, PowerType<Seconds, -2, 1>>>,
        Pack<PowerType<Meters, -1, 1>, PowerType<Seconds, 2, 1>>>);

    static_assert(std::is_same_v<
        power_pack_add_t<Pack<PowerType<Meters, 1, 1>, PowerType<Seconds, -2, 1>>, Pack<PowerType<Meters, 1, 1>, PowerType<Seconds, -2, 1>>>,
        Pack<PowerType<Meters, 2, 1>, PowerType<Seconds, -4, 1>>>);

    static_assert(std::is_same_v<
        power_pack_add_t<Pack<EmptyPack>, Pack<EmptyPack>>,
        Pack<EmptyPack>>);

    static_assert(std::is_same_v<
        power_pack_mult_t<Pack<PowerType<Meters, 1, 1>, PowerType<Seconds, -2, 1>>, Pack<PowerType<Meters, 1, 1>, PowerType<Seconds, -2, 1>>>,
        Pack<PowerType<Meters, 1, 1>, PowerType<Seconds, 4, 1>>>);

    static_assert(std::is_same_v<
        raise_power_pack_t<Pack<PowerType<Meters, 1, 1>, PowerType<Seconds, -2, 1>>, 2, 3>,
        Pack<PowerType<Meters, 2, 3>, PowerType<Seconds, -4, 3>>>);

    constexpr auto x = dist_t<Length>(20);
    constexpr auto y = dist_t<Width>(10);
    constexpr auto z = dist_t<Depth>(5);

    constexpr auto w = foo(x * y * z);
    static_assert(std::is_same_v<std::__remove_cvref_t<decltype(w)>, area_t>);

    constexpr auto w2 = foo(x * x * y);
    static_assert(std::is_same_v<std::__remove_cvref_t<decltype(w2)>, char>);

    static_assert(std::is_same_v<decltype(semantic_cast<NoSemanticType>(x)), 
        Unit<double, 1, NoSemanticType, Pack<PowerType<Meters, 1, 1>>>>);


}