#ifndef EMP_TYPE_PACK_HPP
#define EMP_TYPE_PACK_HPP
#include <cstddef>
#include <tuple>
namespace emp {
template <typename... Types> struct TypePack {
    static constexpr std::size_t size = sizeof...(Types);

    template <std::size_t Index> using Type = std::tuple_element_t<Index, std::tuple<Types...>>;
};
};
#endif  //  EMP_TYPE_PACK_HPP
