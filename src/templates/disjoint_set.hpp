#ifndef EMP_DISJOINT_SET_HPP
#define EMP_DISJOINT_SET_HPP
#include <stack>
#include <vector>
#include "debug/log.hpp"
namespace emp {
template <std::size_t Size> struct DisjointSet {
    int parent[Size];
    int rank[Size] { 0 };
    void isolate(int element)
    {
        int group_head = group(element);
        rank[group_head] -= 1;
        rank[element] = 0;
        parent[element] = element;
    }
    int group(int element)
    {
        std::stack<int> to_correct;
        while(parent[element] != element) {
            to_correct.push(element);
            element = parent[element];
        }
        while(!to_correct.empty()) {
            auto idx = to_correct.top();
            parent[idx] = element;
            to_correct.pop();
        }
        return element;
    }
    bool isHead(int element) { return element == group(element); }
    void merge(int element1, int element2)
    {
        int group1 = group(element1);
        int group2 = group(element2);
        if(group1 == group2) {
            return;
        }

        int bigger_group = (rank[group1] > rank[group2] ? group1 : group2);
        int smaller_group = (bigger_group == group1 ? group2 : group1);

        rank[bigger_group] += rank[smaller_group] + 1;
        parent[smaller_group] = bigger_group;
    }
    DisjointSet()
    {
        for(int i = 0; i < Size; i++) {
            parent[i] = i;
        }
    }
};
};
#endif  //  EMP_DISJOINT_SET_HPP
