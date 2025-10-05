#ifndef EMP_COMPONENT_HPP
#define EMP_COMPONENT_HPP
#include <bitset>
#include <cstdint>
namespace emp {
typedef uint8_t ComponentType;
const ComponentType MAX_COMPONENTS = 32;
typedef std::bitset<MAX_COMPONENTS> Signature;
};
#endif
