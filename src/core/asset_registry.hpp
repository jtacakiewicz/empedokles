#ifndef EMP_ASSET_REGISTRY_HPP
#define EMP_ASSET_REGISTRY_HPP
#include <memory>
#include <string>
#include <unordered_map>
#include <cassert>
namespace emp {
template <class T, class Id_t = std::string> class AssetRegistry {
    std::unordered_map<Id_t, std::unique_ptr<T>> m_assets;

public:
    template <class... Args> T &create(Id_t id, Args... args)
    {
        auto asset = std::make_unique<T>(args...);
        assert(!m_assets.contains(id) && "trying to override existing asset");
        m_assets[id] = std::move(asset);
        return *m_assets.at(id);
    }
    bool isLoaded(Id_t id) const { return m_assets.contains(id); }
    T &get(Id_t id)
    {
        assert(m_assets.contains(id) && "trying to use not loaded asset");
        return *m_assets.at(id);
    }
    const T &get(Id_t id) const
    {
        assert(m_assets.contains(id) && "trying to use not loaded asset");
        return *m_assets.at(id);
    }

    void remove(Id_t id) { m_assets.erase(id); }
    void removeAll() { m_assets.clear(); }
};
};
#endif
