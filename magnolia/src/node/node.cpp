#include "node/node.hpp"

#include "core/logger.hpp"

namespace mag
{
    // Base node
    Node::Node() { LOG_INFO("Node created"); }

    Node::~Node()
    {
        parent = nullptr;
        LOG_INFO("Node destroyed");
    }

    void Node::ready() { LOG_INFO("Node ready"); }
};  // namespace mag
