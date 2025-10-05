#include "tree-view.hpp"
namespace emp {
void TreeView::log(TreeNode &node, int indent)
{
    for(auto &child : node.children) {
        std::string tab = "";
        for(int i = 0; i < indent; i++) {
            tab += '\t';
        }
        EMP_LOG_DEBUG << tab << child.entity;
        log(child, indent + 1);
    }
}
void TreeView::constructTree(Coordinator &ECS)
{
    m_root.children.clear();
    std::stack<std::pair<Entity, TreeNode *>> to_process;
    to_process.push({ ECS.world(), &m_root });

    while(!to_process.empty()) {
        auto [entity, node] = to_process.top();
        to_process.pop();
        auto *trans = ECS.getComponent<Transform>(entity);
        if(trans == nullptr) {
            continue;
        }
        const auto &children = trans->children();
        for(auto child : children) {
            node->children.push_back({ child, {} });
        }
        int i = 0;
        for(auto child : children) {
            to_process.push({ child, &node->children[i++] });
        }
    }
}
void TreeView::drawTreeNode(TreeNode &node, std::function<std::string(Entity)> dispFunc)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushID(node.entity);
    ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
    tree_flags |=
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick;  //  Standard opening mode as we are likely to want to add selection afterwards
    tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;  //  Left arrow support
    if(node.entity == visible_entity) {
        tree_flags |= ImGuiTreeNodeFlags_Selected;
    }
    if(node.children.size() == 0) {
        tree_flags |= ImGuiTreeNodeFlags_Leaf;
    }
    //  DISPLAY COMPONENTS
    std::string name = "entity_" + std::to_string(node.entity);
    if(dispFunc != nullptr) {
        name = dispFunc(node.entity);
    }
    bool node_open = ImGui::TreeNodeEx("", tree_flags, "%s", name.c_str());
    auto prev_visible = visible_entity;
    if(ImGui::IsItemFocused()) {
        visible_entity = node.entity;
    }
    if(prev_visible != visible_entity) {
        just_selected = true;
    }
    if(node_open) {
        for(TreeNode &child : node.children) {
            drawTreeNode(child, dispFunc);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}
void TreeView::draw(const char *title, Coordinator &ECS, std::function<std::string(Entity)> dispFunc)
{
    if(!isOpen) {
        visible_entity = -1;
        return;
    }

    just_selected = false;
    constructTree(ECS);

    if(ImGui::Begin(title, &isOpen)) {
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);

        if(ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg)) {
            for(auto &node : m_root.children) {
                drawTreeNode(node, dispFunc);
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }
}
}
