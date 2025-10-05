#include "particle_system.hpp"
#include "graphics/render_systems/particle_render_system.hpp"
namespace emp {
ParticleSystem::ParticleSystem()
    : m_rndEngine(time(0))
    , m_rndDist(0, 1.f)
{
    m_queue.max_particles = ParticleRenderSystem::MAX_PARTICLE_COUNT;
}
void ParticleSystem::update(float delta_time)
{
    for(auto e : entities) {
        auto &emitter = getComponent<ParticleEmitter>(e);
        if(!emitter.enabled && emitter.enable_for_seconds <= 0.f) {
            continue;
        }
        if(emitter.enable_for_seconds > 0.f) {
            emitter.enable_for_seconds -= std::min(emitter.enable_for_seconds, delta_time);
        }

        auto &transform = getComponent<Transform>(e);

        auto std_count = emitter.count.max() - emitter.count.min();
        auto count = m_rndDist(m_rndEngine) * std_count + emitter.count.min();

        auto color_idx = m_rndDist(m_rndEngine) * emitter.colors.size();
        auto color = emitter.colors.size() == 0 ? vec4f(0) : emitter.colors[color_idx];
        m_queue.emit(count, emitter.type(), transform.position, emitter.type_data(), emitter.speed.range(),
                     emitter.lifetime.range(), { 0.f, EMP_PI * 2.f }, color);
    }
}
void ParticleSystem::compute(const FrameInfo &frame_info, ParticleRenderSystem &render_system)
{
    render_system.compute(frame_info, m_queue);
}
}
