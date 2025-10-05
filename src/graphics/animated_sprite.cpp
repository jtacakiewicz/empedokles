#include "animated_sprite.hpp"
namespace emp {
std::vector<std::unique_ptr<AnimatedSprite::StateMachine_t>> AnimatedSprite::s_state_machines;
MovingSprite MovingSprite::allFrames(Sprite sprite, float whole_time, bool isLooping)
{
    MovingSprite moving;
    moving.isLooping = isLooping;
    moving.sprite = sprite;
    float single_frame_duration = whole_time / static_cast<float>(sprite.frameCount());
    for(int i = 0; i < sprite.frameCount(); i++) {
        moving.frames.push_back({ i, single_frame_duration });
    }
    return moving;
}
MovingSprite MovingSprite::singleFrame(Sprite sprite)
{
    MovingSprite result;
    result.sprite = sprite;
    result.add(0, INFINITY);
    return result;
}
AnimatedSprite::AnimatedSprite(const Builder &builder)
    : m_moving_sprites(builder.moving_sprites)
    , m_anim_state(builder.entry_state)
{
    m_machine_handle = s_state_machines.size();
    s_state_machines.emplace_back(std::make_unique<StateMachine_t>(builder.FSM_builder));
}
void AnimatedSprite::m_processSpriteChange(std::string new_sprite_id)
{
    auto &moving_sprite = m_moving_sprites.at(new_sprite_id);
    auto &sprite = moving_sprite.sprite;

    m_current_anim_frame_idx = 0;
    sprite.frame = moving_sprite.frames[m_current_anim_frame_idx].frame;
    m_current_frame_lasted_sec = 0.f;
}
void AnimatedSprite::m_checkFrameSwitching(float delta_time)
{
    m_current_frame_lasted_sec += delta_time * animation_speed;
    auto &moving_sprite = m_moving_sprites.at(current_sprite_frame());
    auto &sprite = moving_sprite.sprite;
    auto &frames = moving_sprite.frames;
    auto &current_frame = frames[m_current_anim_frame_idx];

    m_current_frame_just_ended = false;

    if(m_current_frame_lasted_sec < current_frame.duration) {
        return;
    }
    m_current_frame_lasted_sec = 0.f;
    m_current_anim_frame_idx++;

    int max_frame_idx = moving_sprite.frames.size();
    if(m_current_anim_frame_idx == max_frame_idx) {
        m_current_frame_just_ended = true;
    }
    if(moving_sprite.isLooping) {
        m_current_anim_frame_idx %= max_frame_idx;
    }
    m_current_anim_frame_idx = std::min(m_current_anim_frame_idx, max_frame_idx - 1);

    sprite.frame = frames[m_current_anim_frame_idx].frame;
}
void AnimatedSprite::updateState(Entity entity, float delta_time)
{
    auto sprite_id_before = current_sprite_frame();
    bool justEnded = m_current_frame_just_ended;
    auto &state_machine = *s_state_machines[m_machine_handle];

    m_anim_state = state_machine.eval(m_anim_state, entity, justEnded);

    auto sprite_id_after = current_sprite_frame();
    if(sprite_id_before != sprite_id_after) {
        m_processSpriteChange(sprite_id_after);
    }
    m_checkFrameSwitching(delta_time);
}
typedef AnimatedSprite::Builder Builder;
Builder::Builder(std::string entry_point, const MovingSprite &default_sprite)
{
    entry_state = entry_point;

    FSM_builder.addNode(entry_state);
    moving_sprites[entry_point] = default_sprite;
}
void Builder::addNode(std::string name, const MovingSprite &sprite)
{
    FSM_builder.addNode(name);
    moving_sprites[name] = sprite;
}
/**
 * @param trigger is a function that return boolean and takes in
 *   Entity and bool value informing if the frame just ended
 */
void Builder::addEdge(std::string from, std::string to, std::function<bool(Entity, bool)> trigger)
{
    FSM_builder.addEdge(from, to, trigger);
}
void Builder::addEdge(std::string from, std::string to, std::function<bool(Entity)> trigger)
{
    FSM_builder.addEdge(from, to, [=](Entity e, bool) -> bool { return trigger(e); });
}
void Builder::addEdge(std::string from, std::string to)
{
    FSM_builder.addEdge(from, to, [](Entity, bool hasEnded) -> bool { return hasEnded; });
}
};  //  namespace emp
