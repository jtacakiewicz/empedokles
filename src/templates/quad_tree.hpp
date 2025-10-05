#ifndef EMP_QUAD_TREE_HPP
#define EMP_QUAD_TREE_HPP
#include <cstdint>
#include <functional>
#include <map>
#include <stack>
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
#include "math/shapes/AABB.hpp"
#include "templates/free_list.hpp"
namespace emp {
template <typename T, typename GetAABB, typename Equal = std::equal_to<T>, typename Float = float> class QuadTree {
    static_assert(std::is_convertible_v<std::invoke_result_t<GetAABB, const T &>, AABB>,
                  "GetAABB must be a callable of signature AABB<Float>(const T&)");
    static_assert(std::is_convertible_v<std::invoke_result_t<Equal, const T &, const T &>, bool>,
                  "Equal must be a callable of signature bool(const T&, const T&)");
    static_assert(std::is_arithmetic_v<Float>);

    typedef uint16_t Int;
    static constexpr Int invalid = -1;

public:
    QuadTree(const AABB &box, const GetAABB &getAABB = GetAABB(), const Equal &equal = Equal())
        : m_box(box)
        , m_getAABB(getAABB)
        , m_equal(equal)
    {
        m_nodes.insert({ invalid });
    }

    void add(const T &value) { add(m_root, 0, m_box, value); }

    void remove(const T &value) { remove(m_root, m_box, value); }

    std::vector<T> query(const AABB &box) const
    {
        auto values = std::vector<T>();
        query(m_root, m_box, box, values);
        return values;
    }
    void update(T value)
    {
        //  auto itr = _locations.find(value);
        //  if(itr != _locations.end()) {
        //      auto b = itr->second->box;
        //      if(AABBcontainsAABB(b, _getAABB(value)))
        //          return;
        //      remove(value);
        //      add(value);
        //
        //  } else {
        //      add(value);
        //  }
    }

    std::vector<std::pair<T, T>> findAllIntersections() const { return findAllIntersections(m_root); }

    AABB getAABB() const { return m_box; }
    void updateLeafes() { updateLeafes(m_root); }
    void clear() { clear(m_root); }

private:
    static constexpr size_t threshold = 8;
    static constexpr size_t max_depth = 8;

    struct Node {
        Int first_child = invalid;
        Int first_elem = invalid;
        int count = 0;
    };
    struct Element {
        T value;
        Int next;
    };

    FreeList<Element> m_elements;
    AABB m_box;
    static constexpr int m_root = 0U;
    FreeList<Node> m_nodes;

    GetAABB m_getAABB;
    Equal m_equal;

    void insertElem(int node_idx, int elem_idx)
    {
        int prev = m_nodes[node_idx].first_elem;
        m_nodes[node_idx].first_elem = elem_idx;
        m_elements[elem_idx].next = prev;
        m_nodes[node_idx].count++;
    }
    void removeElem(int node_idx, int elem_idx)
    {
        int prev = invalid;
        int cur_idx = m_nodes[node_idx].first_elem;
        if(cur_idx == elem_idx) {
            m_nodes[node_idx].first_elem = m_elements[cur_idx].next;
            return;
        }
        while(cur_idx != invalid && m_elements[cur_idx].next != elem_idx) {
            cur_idx = m_elements[cur_idx].next;
        }
        if(cur_idx == invalid) {
            return;
        }
        m_elements[cur_idx].next = m_elements[elem_idx].next;
        m_elements[elem_idx].next = invalid;
        m_nodes[node_idx].count--;
    }
    bool isLeaf(int node) const { return (m_nodes[node].first_child == invalid); }

    AABB computeAABB(const AABB &box, int i) const
    {
        auto origin = box.min;
        auto childSize = box.size() / static_cast<Float>(2);
        switch(i) {
            case 0:
                return AABB::CreateMinSize(origin, childSize);
            case 1:
                return AABB::CreateMinSize(vec2f(origin.x + childSize.x, origin.y), childSize);
            case 2:
                return AABB::CreateMinSize(vec2f(origin.x, origin.y + childSize.y), childSize);
            case 3:
                return AABB::CreateMinSize(origin + childSize, childSize);
            default:
                assert(false && "Invalid child index");
                return AABB();
        }
    }

    int getQuadrant(const AABB &nodeAABB, const AABB &valueAABB) const
    {
        auto center = nodeAABB.center();
        if(valueAABB.right() < center.x) {
            if(valueAABB.top() < center.y) {
                return 0;
            } else if(valueAABB.bottom() >= center.y) {
                return 2;
            } else {
                return invalid;
            }
        } else if(valueAABB.left() >= center.x) {
            if(valueAABB.top() < center.y) {
                return 1;
            } else if(valueAABB.bottom() >= center.y) {
                return 3;
            } else {
                return invalid;
            }
        } else {
            return invalid;
        }
    }

    int add(const int node_idx, size_t depth, const AABB &box, const T &value)
    {
        assert(node_idx != invalid);
        assert(AABBcontainsAABB(box, m_getAABB(value)));
        if(isLeaf(node_idx)) {
            if(depth >= max_depth || m_nodes[node_idx].count < threshold) {
                insertElem(node_idx, m_elements.insert({ value, invalid }));
                return node_idx;
            } else {
                split(node_idx, box);
                return add(node_idx, depth, box, value);
            }
        }
        auto quadrant = getQuadrant(box, m_getAABB(value));
        if(quadrant == invalid) {
            insertElem(node_idx, m_elements.insert({ value, invalid }));
            return node_idx;
        } else {
            int child_idx = m_nodes[node_idx].first_child + quadrant;
            return add(child_idx, depth + 1, computeAABB(box, quadrant), value);
        }
    }

    void split(int node_idx, const AABB &box)
    {
        assert(node_idx != invalid);
        assert(isLeaf(node_idx) && "Only leaves can be split");
        int quad = 0;
        int first_child = 0xffffff;
        for(int i = 0; i < 4; i++) {
            auto new_child = m_nodes.insert({ invalid, invalid });
            first_child = std::min(new_child, first_child);
        }
        m_nodes[node_idx].first_child = first_child;

        int elem_idx = m_nodes[node_idx].first_elem;
        m_nodes[node_idx].first_elem = invalid;
        m_nodes[node_idx].count = 0;

        while(elem_idx != invalid) {
            int cur_idx = elem_idx;
            elem_idx = m_elements[cur_idx].next;

            const auto &value = m_elements[cur_idx].value;
            auto quadrant = getQuadrant(box, m_getAABB(value));
            if(quadrant != invalid) {
                int child_idx = quadrant + m_nodes[node_idx].first_child;
                insertElem(child_idx, cur_idx);
            } else {
                insertElem(node_idx, cur_idx);
            }
        }
    }

    bool remove(int node_idx, const AABB &box, const T &value)
    {
        assert(node_idx != invalid);
        assert(AABBcontainsAABB(box, m_getAABB(value)));
        if(isLeaf(node_idx)) {
            removeValue(node_idx, value);
            return true;
        } else {
            auto quadrant = getQuadrant(box, m_getAABB(value));
            if(quadrant != invalid) {
                int child_idx = m_nodes[node_idx].first_child + quadrant;
                return remove(child_idx, computeAABB(box, quadrant), value);
            } else {
                removeValue(node_idx, value);
                return node_idx;
            }
            return false;
        }
    }

    void removeValue(int node_idx, const T &value)
    {
        int elem_idx = m_nodes[node_idx].first_elem;
        while(elem_idx != invalid && m_elements[elem_idx].value != value) {
            elem_idx = m_elements[elem_idx].next;
        }
        if(elem_idx == invalid) {
            return;
        }
        removeElem(node_idx, elem_idx);
        m_elements.erase(elem_idx);
    }

    bool tryMerge(int node_idx)
    {
        assert(node_idx != invalid);
        assert(!isLeaf(node_idx) && "Only interior nodes can be merged");
        auto vals_count = m_nodes[node_idx].count;
        for(int i = 0; i < 4; i++) {
            int child_idx = m_nodes[node_idx].first_child + i;
            if(!isLeaf(child_idx)) {
                return false;
            }
            vals_count += m_nodes[child_idx].count;
        }
        if(vals_count <= threshold) {
            for(int i = 0; i < 4; i++) {
                int child_idx = m_nodes[node_idx].first_child + i;

                int elem_idx = m_nodes[child_idx].first_elem;
                m_nodes[child_idx].first_elem = invalid;
                m_nodes[child_idx].count = 0;

                while(elem_idx != invalid) {
                    int cur_idx = elem_idx;
                    elem_idx = m_elements[elem_idx].next;
                    insertElem(node_idx, cur_idx);
                }
            }
            for(int i = 0; i < 4; i++) {
                int child_idx = m_nodes[node_idx].first_child + i;
                m_nodes.erase(child_idx);
            }
            m_nodes[node_idx].first_child = invalid;
            return true;
        } else {
            return false;
        }
    }
    void updateLeafes(int node_idx)
    {
        auto isEndBranch = isLeaf(node_idx) || tryMerge(node_idx);
        if(isEndBranch) {
            return;
        }
        auto first = m_nodes[node_idx].first_child;
        for(int i = 0; i < 4; i++) {
            int child_idx = m_nodes[node_idx].first_child + i;
            if(child_idx == node_idx) {
                exit(0);
            }
            updateLeafes(child_idx);
        }
    }
    void clear(int node_idx)
    {
        if(node_idx == invalid) {
            return;
        }
        int elem_idx = m_nodes[node_idx].first_elem;
        std::vector<int> elems;
        while(elem_idx != invalid) {
            elems.push_back(elem_idx);
            elem_idx = m_elements[elem_idx].next;
        }
        for(auto e : elems) {
            m_elements.erase(e);
        }
        m_nodes[node_idx].first_elem = invalid;
        m_nodes[node_idx].count = 0;

        if(isLeaf(node_idx)) {
            return;
        }
        for(int i = 0; i < 4; i++) {
            int child_idx = m_nodes[node_idx].first_child + i;
            clear(child_idx);
        }
    }

    void query(int node_idx, const AABB &box, const AABB &queryAABB, std::vector<T> &values) const
    {
        assert(node_idx != invalid);
        if(!isOverlappingAABBAABB(queryAABB, box)) {
            return;
        }
        int elem_idx = m_nodes[node_idx].first_elem;
        while(elem_idx != invalid) {
            const auto &value = m_elements[elem_idx].value;
            if(isOverlappingAABBAABB(queryAABB, m_getAABB(value))) {
                values.push_back(value);
            }
            elem_idx = m_elements[elem_idx].next;
        }
        if(!isLeaf(node_idx)) {
            for(int i = 0; i < 4; i++) {
                auto childAABB = computeAABB(box, static_cast<int>(i));
                auto child_idx = m_nodes[node_idx].first_child + i;
                if(isOverlappingAABBAABB(queryAABB, childAABB)) {
                    query(child_idx, childAABB, queryAABB, values);
                }
            }
        }
    }

    void searchIntersecionsInNode(int node_idx, int node_depth, std::vector<std::pair<T, T>> &intersections,
                                  std::array<int, max_depth> &parent_stack) const
    {
        if(node_idx == invalid) {
            return;
        }

        //  all elements in the same node
        for(int elem1_idx = m_nodes[node_idx].first_elem; elem1_idx != invalid;) {
            const auto &value1 = m_elements[elem1_idx].value;
            for(int elem2_idx = m_nodes[node_idx].first_elem; elem2_idx != invalid && elem2_idx != elem1_idx;) {
                const auto &value2 = m_elements[elem2_idx].value;

                if(isOverlappingAABBAABB(m_getAABB(value1), m_getAABB(value2))) {
                    intersections.emplace_back(value1, value2);
                }

                elem2_idx = m_elements[elem2_idx].next;
            }
            elem1_idx = m_elements[elem1_idx].next;
        }
        for(int cur_depth = 0; cur_depth < node_depth; cur_depth++) {
            auto parent_idx = parent_stack[cur_depth];
            assert(parent_idx != invalid);

            for(int elem1_idx = m_nodes[parent_idx].first_elem; elem1_idx != invalid;) {
                const auto &value1 = m_elements[elem1_idx].value;
                int elem2_idx = m_nodes[node_idx].first_elem;
                while(elem2_idx != invalid) {
                    const auto &value2 = m_elements[elem2_idx].value;
                    if(isOverlappingAABBAABB(m_getAABB(value1), m_getAABB(value2))) {
                        intersections.emplace_back(value1, value2);
                    }
                    elem2_idx = m_elements[elem2_idx].next;
                }
                elem1_idx = m_elements[elem1_idx].next;
            }
        }
    }
    std::vector<std::pair<T, T>> findAllIntersections(int root_idx) const
    {
        struct NodeInfo {
            int index;
            int depth;
        };
        std::vector<std::pair<T, T>> intersections;

        std::stack<NodeInfo> to_process;
        std::array<int, max_depth> parent_stack;
        to_process.push({ root_idx, 0 });
        while(!to_process.empty()) {
            auto node = to_process.top();
            to_process.pop();
            if(node.index == invalid) {
                continue;
            };
            searchIntersecionsInNode(node.index, node.depth, intersections, parent_stack);
            if(isLeaf(node.index)) {
                continue;
            }
            parent_stack[node.depth] = node.index;
            if(isLeaf(node.index)) {
                continue;
            }

            int first_child_idx = m_nodes[node.index].first_child;
            for(int i = 0; i < 4; i++) {
                to_process.push({ first_child_idx + i, node.depth + 1 });
            }
        }
        return intersections;
    }
};

}
#endif
