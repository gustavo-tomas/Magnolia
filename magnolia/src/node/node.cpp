#include "node/node.hpp"

#include "core/logger.hpp"

namespace mag
{
    // Base node
    Node::Node() { LOG_INFO("Node created"); }

    Node::~Node()
    {
        parent = nullptr;
        for (const auto& child : children) delete child;

        LOG_INFO("Node destroyed");
    }

    void Node::ready() { LOG_INFO("Node ready"); }

    void Node::set_name(const str& name) { this->name = name; }
    void Node::set_parent(Node* parent) { this->parent = parent; }
    void Node::add_child(Node* child) { this->children.push_back(child); }
};  // namespace mag
