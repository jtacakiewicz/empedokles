#ifndef EMP_PARTICLE_SYSTEM_HPP
#define EMP_PARTICLE_SYSTEM_HPP
#include <cstdint>
#include <optional>
#include <random>
#include <utility>
#include "core/system.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/particle_emit_queue.hpp"
#include "graphics/render_systems/particle_render_system.hpp"
#include "math/shapes/AABB.hpp"
#include "scene/transform.hpp"
namespace emp {
struct ParticleEmitter {
private:
    eEmitType m_type = eEmitType::POINT;
    std::pair<float, float> m_type_data;

public:
    eEmitType type() const { return m_type; }
    std::pair<float, float> type_data() const { return m_type_data; }
    template <class T> struct OptionalRange {
    private:
        bool isRangeSet = false;
        T m_min;
        T m_max;

    public:
        void setMin(const T &v)
        {
            isRangeSet = true;
            m_min = v;
        }
        void setValue(const T &v)
        {
            m_min = v;
            m_max = v;
            isRangeSet = false;
        }
        void setMax(const T &v)
        {
            isRangeSet = true;
            m_max = v;
        }
        T min() const { return m_min; }
        T max() const
        {
            if(isRangeSet) {
                return m_max;
            }
            return m_min;
        }
        std::pair<T, T> range() const { return { min(), max() }; }
        OptionalRange(const T &mi, const T &ma)
            : m_min(mi)
            , m_max(ma)
            , isRangeSet(true)
        {
        }
        OptionalRange(const T &v)
            : m_min(v)
            , isRangeSet(false)
        {
        }
        OptionalRange()
            : isRangeSet(false)
        {
        }
    };

    OptionalRange<uint32_t> count;
    OptionalRange<float> speed;
    OptionalRange<float> lifetime;
    std::vector<vec4f> colors;

    void setRing(float min_dist, float max_dist)
    {
        m_type_data = { min_dist, max_dist };
        m_type = eEmitType::RING;
    }
    void setLine(vec2f direction)
    {
        m_type_data = { direction.x, direction.y };
        m_type = eEmitType::LINE;
    }
    void setRect(vec2f size)
    {
        m_type_data = { size.x, size.y };
        m_type = eEmitType::RECT;
    }
    //  emits if enabled is true or if enableFor is set to some value
    bool enabled = false;
    float enable_for_seconds = 0.f;
};
class ParticleSystem : public System<ParticleEmitter, Transform> {
    EmitQueue m_queue;
    std::default_random_engine m_rndEngine;
    std::uniform_real_distribution<float> m_rndDist;

public:
    void update(float delta_time);
    void compute(const FrameInfo &, ParticleRenderSystem &render_system);
    ParticleSystem();
};
}
#endif
