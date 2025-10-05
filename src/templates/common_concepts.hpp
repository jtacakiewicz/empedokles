#ifndef EMP_COMMON_CONCEPTS
#define EMP_COMMON_CONCEPTS
#include <concepts>
#include <limits>
#include <vector>
#include "math/math_defs.hpp"

namespace emp {
template <typename T>
concept IsVec2f = requires(const T &v) {
    { decltype(v.x)(v.x) } -> std::same_as<float>;
    { decltype(v.y)(v.y) } -> std::same_as<float>;
};
static_assert(IsVec2f<vec2f>);

template <typename C>
concept IterableContainerOfVec2f = requires(const C &c) {
    typename C::value_type;
    requires IsVec2f<typename C::value_type>;
    { begin(c) } -> std::input_or_output_iterator;
    { end(c) } -> std::input_or_output_iterator;
};
static_assert(IterableContainerOfVec2f<std::vector<vec2f>>);
};

#endif
