//
// Created by Samarth Mahendra on 2/27/26.
//

#ifndef CLUSTER_SCHEDULER_H
#define CLUSTER_SCHEDULER_H
#pragma once

#include <unordered_map>
#include <queue>
#include <mutex>
#include <optional>
#include <string>

#include "../cluster/node_registry.h"
#include "../rpc/node_client.h"

#include "../../core/task.h"
#include "../../core/object_ref.h"

namespace orion::distributed {

    // Cluster-level scheduler:
    // - chooses nodes
    // - dispatches tasks
    // - tracks object locations
    class ClusterScheduler {
    public:
        ClusterScheduler(NodeRegistry& registry, NodeClient& client);

        // Submit a task to the cluster (may or may not dispatch immediately).
        // Returns ObjectRef for the output object (id == task.id).
        orion::ObjectRef submit(orion::Task task);

        // Try to dispatch any runnable tasks.
        void schedule();

        // Record that an object is available on some node.
        // (In v0.2, we can "predict" outputs at dispatch time; later nodes will report.)
        void on_object_created(const std::string& object_id, const std::string& node_id);

        // Where does this object live?
        std::optional<std::string> object_location(const std::string& object_id);

    private:
        bool deps_ready_(const orion::Task& task) const;

    private:
        NodeRegistry& registry_;
        NodeClient& client_;

        // object_id -> node_id
        std::unordered_map<std::string, std::string> object_locations_;

        // tasks waiting for deps
        std::queue<orion::Task> pending_;

        mutable std::mutex mu_;
    };

} // namespace orion::distributed


#endif //CLUSTER_SCHEDULER_H
