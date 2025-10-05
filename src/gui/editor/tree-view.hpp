#ifndef EMP_TREE_VIEW_HPP
#define EMP_TREE_VIEW_HPP

#include "imgui.h"
#include <stack>
#include <string>
#include <vector>
#include "core/coordinator.hpp"
#include "core/entity.hpp"
#include "debug/log.hpp"
#include "scene/transform.hpp"
namespace emp {
class TreeView {
private:
    struct TreeNode {
        Entity entity;
        std::vector<TreeNode> children;
    };
    TreeNode m_root;
    Entity visible_entity = -1;
    bool just_selected = false;

    void log(TreeNode &node, int indent = 0);
    void constructTree(Coordinator &ECS);
    void drawTreeNode(TreeNode &node, std::function<std::string(Entity)> dispFunc);

public:
    Entity getSelected() const { return visible_entity; }
    bool isJustSelected() const { return just_selected; }
    bool isOpen = true;
    void log() { log(m_root, 1); }
    void draw(const char *title, Coordinator &ECS, std::function<std::string(Entity)> dispFunc = nullptr);
    TreeView() { }
};

}
#endif  //  EMP_TREE_VIEW_HPP
