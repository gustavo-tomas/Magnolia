#pragma once

#include <vector>

#include "core/types.hpp"

namespace mag
{
    class Node
    {
        public:
            virtual ~Node();

            // Method to call when the node is added to the scene
            virtual void ready();

            void set_name(const str& name);
            void set_parent(Node* parent);
            void add_child(Node* child);

            const str& get_name() const { return name; };
            Node* get_parent() { return parent; };
            std::vector<Node*>& get_children() { return children; };

        protected:
            Node();

            Node* parent = {};  // A pointer to the parent Node
            std::vector<Node*> children = {};
            str name = "Node";
    };
};  // namespace mag
