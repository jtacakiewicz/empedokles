#ifndef EMP_FINITE_STATE_MACHINE
#define EMP_FINITE_STATE_MACHINE

#include <string>
#include <functional>
#include <unordered_set>
#include <vector>
#include <cassert>
#include <memory>

namespace emp {
template <class Identifier_t, class... TriggerFuncInput_t> class FiniteStateMachine {
    typedef std::function<bool(TriggerFuncInput_t...)> TriggerFunc_t;

    struct Edge;
    struct Node {
        Identifier_t name;
        std::vector<Edge> outgoing_edges;

        Node() { }
        Node(Identifier_t id)
            : name(id)
        {
        }
    };
    struct Edge {
        Identifier_t from;
        Identifier_t to;
        TriggerFunc_t trigger;
        Edge(Identifier_t from_, Identifier_t to_, TriggerFunc_t trigger_)
            : from(from_)
            , to(to_)
            , trigger(trigger_)
        {
        }
    };

    std::unordered_map<Identifier_t, Node> m_nodes;

public:
    class Builder {
    protected:
        std::unordered_map<Identifier_t, Node> nodes;

    public:
        void addNode(Identifier_t name)
        {
            assert(!nodes.contains(name) && "cannot have more than 1 node of the same name in state machine");
            nodes[name] = Node(name);
        }
        void addEdge(Identifier_t from, Identifier_t to, TriggerFunc_t trigger)
        {
            assert(nodes.contains(from) && "tried to create edge without start");
            assert(nodes.contains(to) && "tried to create edge without end");

            Identifier_t from_ptr = nodes.at(from).name;
            Identifier_t to_ptr = nodes.at(to).name;
            nodes.at(from).outgoing_edges.push_back(Edge(from_ptr, to_ptr, trigger));
        }
        friend FiniteStateMachine;
    };

    Identifier_t eval(Identifier_t current_state, TriggerFuncInput_t... input) const
    {
        Identifier_t prev_node;

        constexpr int max_iter_count = 100;
        for(int iter = 0; iter < max_iter_count; iter++) {
            prev_node = current_state;
            assert(m_nodes.contains(current_state) && "empty FSM impossible");
            auto &node = m_nodes.at(current_state);
            for(const auto &edge : node.outgoing_edges) {
                assert(edge.from == current_state && "this must be true");
                if(edge.trigger(input...) == true) {
                    current_state = edge.to;
                    break;
                }
            }
            //  if node didnt change
            if(current_state == prev_node) {
                break;
            }
        }
        return current_state;
    }
    FiniteStateMachine() = delete;
    FiniteStateMachine(const Builder &builder)
        : m_nodes(builder.nodes)
    {
    }
};
};
#endif  //  EMP_FINITE_STATE_MACHINE
