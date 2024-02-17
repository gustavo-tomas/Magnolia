#pragma once

namespace mag
{
    class Node
    {
        public:
            virtual ~Node();

            // Method to call when the node is added to the scene
            virtual void ready();

        protected:
            Node();

            Node *parent = {};  // A pointer to the parent Node
    };
};  // namespace mag
