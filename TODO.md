# 🎯 TODO:

- sound
    - basic sound asset system
    - simple 'play sample'
- graphics
    - debug shapes improved
    - instanced rendering for sprites
    - 2d lighting
- core
    - serialization
    - message bus
- collision
    - optimize to not be limited by physics system
    - more constraints+
        - piston
- networking?

---

# 📌 DOING:

- core
    - correct transform resolution

---

# ⚠️ ISSUES:
- weird error message when closing window
- when spawing overlapping colliders they explode
- 'drifting' physics problem
- semaphore problem `vkQueueSubmit(): pSubmits[0].pSignalSemaphores[0] (VkSemaphore 0x1e000000001e) is being signaled by VkQueue`
---

# ✅ DONE:

- collision
    - added more constraints
        - anchored swivel
        - unanchored swivel
        - anchored fixed
        - unanchored fixed
- graphics
    - imgui inspector for basic components
    - basic animation system
        - truly moving animated sprites
    - add imgui
- collision
    - collision optimization
        - collision idle state
    - collision triggers
- graphics
    - gui - mainly for debuggin for now
    - resizable window
    - render system abstraction
    - basic animation system
        - animated sprites
        - finite state machine animated sprite switching
- separate threads for rendering, physics and the rest
- constraints as components
- sprites (color, texRect, origin, render pipeline)

- ##### basics of rendering system
- ##### basics of collision system
- ##### basics of ECS system
