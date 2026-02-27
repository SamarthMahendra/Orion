//
// Created by Samarth Mahendra on 2/27/26.
//

#ifndef NODE_CLIENT_H
#define NODE_CLIENT_H

#pragma once

#include <string>
#include <memory>

#include "../../core/task.h"
#include "../../core/object_ref.h"

namespace orion::distributed {

    // Abstract client: "send a task to a node"
    class NodeClient {
    public:
        virtual ~NodeClient() = default;

        // Fire-and-forget execution request. Returns the ObjectRef of task output.
        virtual orion::ObjectRef submit_task(const std::string& node_id,
                                             orion::Task task) = 0;
    };

} // namespace orion::distributed

#endif //NODE_CLIENT_H
