#ifndef EMP_MATERIAL_HPP
#define EMP_MATERIAL_HPP
namespace emp {
class Material;
struct Material {
    float static_friction = 0.3;
    float dynamic_friction = 0.3;
    float restitution = 0.1;
    float air_friction = 5.0f;
};
};  //  namespace emp
#endif
