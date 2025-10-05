#ifndef EMP_SWEEP_LINE_HPP
#define EMP_SWEEP_LINE_HPP
#include <algorithm>
#include <functional>
#include <vector>
#include "math/geometry_func.hpp"
#include "math/shapes/AABB.hpp"

namespace emp {
template <class T, class Iterator>
std::vector<std::pair<T, T>> sweepLine(
    Iterator begin, Iterator end, std::function<AABB(const T &)> getAABB,
    std::function<bool(const T &, const T &)> filter = [](const T &, const T &) { return true; })
{
    std::vector<std::pair<T, T>> result;
    struct Object {
        Iterator itr;
        AABB aabb;
    };
    std::vector<Object> objects_sorted;
    for(auto itr = begin; itr != end; itr++) {
        objects_sorted.push_back({ itr, getAABB(*itr) });
    }
    std::sort(objects_sorted.begin(), objects_sorted.end(),
              [](const Object &a, const Object &b) -> bool { return a.aabb.min.x < b.aabb.min.x; });

    std::vector<size_t> opened;
    for(auto i = 0; i < objects_sorted.size(); ++i) {
        for(int j = 0; j < opened.size(); j++) {
            auto op_idx = opened[j];
            if(objects_sorted[i].aabb.min.x > objects_sorted[op_idx].aabb.max.x) {
                opened.erase(opened.begin() + j);
                j--;
            } else {
                const auto &object_added = objects_sorted[i];
                const auto &object_opened = objects_sorted[op_idx];
                if(!filter(*object_added.itr, *object_opened.itr)) {
                    continue;
                }
                if(isOverlappingAABBAABB(object_added.aabb, object_opened.aabb)) {
                    result.push_back({ *object_added.itr, *object_opened.itr });
                }
            }
        }
        opened.push_back(i);
    }
    return result;
}
};  //  namespace emp
#endif
