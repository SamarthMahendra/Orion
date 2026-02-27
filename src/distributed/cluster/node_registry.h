//
// Created by Samarth Mahendra on 2/27/26.
//

#ifndef NODE_REGISTRY_H
#define NODE_REGISTRY_H



#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <optional>

namespace orion::distributed {

    struct NodeInfo {
        std::string node_id;
        std::string address;      // "host:port"
        int available_workers;    // simple resource metric
        bool alive = true;
    };

    class NodeRegistry {
    public:
        // Add or update node
        void register_node(const NodeInfo& node);

        // Remove node (or mark dead)
        void remove_node(const std::string& node_id);

        // Mark node heartbeat
        void heartbeat(const std::string& node_id);

        // List all nodes
        std::vector<NodeInfo> nodes();

        // Pick a node for scheduling (round-robin)
        std::optional<NodeInfo> pick_node();

    private:
        std::unordered_map<std::string, NodeInfo> nodes_;
        std::mutex mutex_;

        size_t rr_index_ = 0;   // round robin pointer
    };

} // namespace orion::distributed


#endif //NODE_REGISTRY_H
