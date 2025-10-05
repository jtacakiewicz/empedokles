#ifndef EMP_LAYER_HPP
#define EMP_LAYER_HPP
#include <bitset>
#include <cstdint>
namespace emp {

const unsigned int MAX_LAYERS = 32;
typedef std::uint8_t Layer;
const Layer INVALID_LAYER = UINT8_MAX;
typedef std::bitset<MAX_LAYERS> LayerMask;

};
#endif  //  EMP_LAYEr_HPP
