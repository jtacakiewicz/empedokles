#ifndef EMP_PARTICLE_EMIT_QUEUE_HPP
#define EMP_PARTICLE_EMIT_QUEUE_HPP
#include <cstdint>
#include "math/math_defs.hpp"
namespace emp {
static constexpr uint32_t MAX_EMIT_CALLS = 16U;
enum class eEmitType : uint32_t {
    POINT = 0,
    LINE = 1,
    RING = 2,
    RECT = 3,
};
struct alignas(16) ParticleEmitData {
    uint32_t count;  //  4, 4
    uint32_t type;   //  4, 8
    vec2f position;  //  8, 16
    union {
        vec2f line_direction;
        struct {
            float ring_dist_min;
            float ring_dist_max;
        } ring;
        float type_data[2];
    };  //  8, 24

    float speed_min;  //  4, 28
    float speed_max;  //  4, 32

    float lifetime_min;  //  4, 36
    float lifetime_max;  //  4, 40

    float angle_min;  //  4, 44
    float angle_max;  //  4, 48

    //  48 % 16 == 0 C:, all good
    vec4f color;
};
struct alignas(64) EmitQueue {
    ParticleEmitData calls[MAX_EMIT_CALLS];  //  16*16
    uint32_t call_count = 0;                 //  4, 4
    uint32_t work_start = 0;                 //  4, 8
    uint32_t work_end = 0;                   //  4, 12
    uint32_t max_particles;
    void emit(uint32_t part_count, eEmitType type, vec2f pos, std::pair<float, float> type_data, std::pair<float, float> speed,
              std::pair<float, float> lifetime, std::pair<float, float> angle, vec4f c)
    {
        if(call_count >= MAX_EMIT_CALLS) {
            return;
        }
        work_end += part_count;
        auto idx = call_count++;
        calls[idx].type = static_cast<uint32_t>(type);
        calls[idx].type_data[0] = type_data.first;
        calls[idx].type_data[1] = type_data.second;
        calls[idx].count = part_count;
        calls[idx].position = pos;
        calls[idx].lifetime_min = lifetime.first;
        calls[idx].lifetime_max = lifetime.second;
        calls[idx].angle_min = angle.first;
        calls[idx].angle_max = angle.second;
        calls[idx].speed_min = speed.first;
        calls[idx].speed_max = speed.second;
        calls[idx].color = c;
    }
    void reset()
    {
        work_start = work_end;
        call_count = 0;
    }
};
}
#endif
