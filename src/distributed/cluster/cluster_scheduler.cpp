//
// Created by Samarth Mahendra on 2/27/26.
//

#include "cluster_scheduler.h"

#include "cluster_scheduler.h"

namespace orion::distributed {

ClusterScheduler::ClusterScheduler(NodeRegistry& registry, NodeClient& client)
    : registry_(registry), client_(client) {}

orion::ObjectRef ClusterScheduler::submit(orion::Task task) {
    orion::ObjectRef out{task.id};

    {
        std::lock_guard<std::mutex> lock(mu_);
        pending_.push(std::move(task));
    }

    // eager scheduling
    schedule();
    return out;
}

void ClusterScheduler::schedule() {
    // We'll do a simple pass:
    // pop tasks, dispatch runnable ones, requeue non-runnable ones.
    std::queue<orion::Task> next_pending;

    while (true) {
        std::optional<orion::Task> task_opt;
        {
            std::lock_guard<std::mutex> lock(mu_);
            if (pending_.empty()) break;

            task_opt.emplace(std::move(pending_.front()));   // ✅ move-construct
            pending_.pop();
        }

        orion::Task task = std::move(*task_opt);

        if (!deps_ready_(task)) {
            next_pending.push(std::move(task));
            continue;
        }

        // pick a node
        auto node_opt = registry_.pick_node();
        if (!node_opt) {
            // no nodes available → keep task pending
            next_pending.push(std::move(task));
            continue;
        }

        const auto& node = *node_opt;

        // Dispatch
        // In v0.2, we assume output object lives on the node we dispatch to.
        // Later, the node will confirm via RPC callback/event.
        // Save id before move — task.id is empty after std::move.
        const std::string task_id = task.id;
        client_.submit_task(node.node_id, std::move(task));

        // Record expected output location optimistically
        on_object_created(task_id, node.node_id);
    }

    // restore pending queue
    {
        std::lock_guard<std::mutex> lock(mu_);
        while (!next_pending.empty()) {
            pending_.push(std::move(next_pending.front()));
            next_pending.pop();
        }
    }
}

void ClusterScheduler::on_object_created(const std::string& object_id,
                                        const std::string& node_id) {
    std::lock_guard<std::mutex> lock(mu_);
    object_locations_[object_id] = node_id;
}

std::optional<std::string> ClusterScheduler::object_location(const std::string& object_id) {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = object_locations_.find(object_id);
    if (it == object_locations_.end()) return std::nullopt;
    return it->second;
}

bool ClusterScheduler::deps_ready_(const orion::Task& task) const {
    std::lock_guard<std::mutex> lock(mu_);
    for (const auto& dep : task.deps) {
        if (object_locations_.find(dep.id) == object_locations_.end()) {
            return false;
        }
    }
    return true;
}

} // namespace orion::distributed