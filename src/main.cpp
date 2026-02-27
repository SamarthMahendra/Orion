//
// Created by Samarth Mahendra on 2/27/26.
//
#include "core/worker.h"
#include "core/task.h"
#include "core/scheduler.h"


#include <iostream>
#include <any>
#include "local/runtime.h"

using namespace orion;
//
// int main() {
//     orion::Runtime rt(2);
//
//     orion::Task t{
//         "A",
//         {},
//         [](std::vector<std::any>) { return 2; }
//     };
//
//     auto ref = rt.submit(t);
//
//     rt.wait(ref);
//
//     std::cout << std::any_cast<int>(rt.get(ref)) << std::endl;
//
//     rt.shutdown();
// }

// #include "distributed/node_runtime.h"
//
// int main() {
//     orion::distributed::NodeRuntime node(4, 50051);
//
//     node.start();
//
//     auto& rt = node.local_runtime();
//
//     orion::Task t{
//         "A",
//         {},
//         [](std::vector<std::any>) { return 42; }
//     };
//
//     auto ref = rt.submit(t);
//     rt.wait(ref);
//
//     node.stop();
// }


#include "distributed/node_runtime.h"
#include "distributed/cluster/node_registry.h"
#include "distributed/cluster/cluster_scheduler.h"
#include "distributed/rpc/inprocess_node_client.h"


int main() {
    using namespace orion::distributed;

    NodeRuntime n1(2, 5001);
    NodeRuntime n2(2, 5002);

    n1.start();
    n2.start();

    NodeRegistry registry;
    registry.register_node({"node-1", "localhost:5001", 2, true});
    registry.register_node({"node-2", "localhost:5002", 2, true});

    InProcessNodeClient client;
    client.add_node("node-1", &n1);
    client.add_node("node-2", &n2);

    ClusterScheduler cluster(registry, client);

    // Task A
    orion::Task t1{
        "A",
        {},
        [](const std::vector<std::any>&) -> std::any { return 10; }
    };

    // Task B depends on A
    orion::Task t2{
        "B",
        {orion::ObjectRef{"A"}},
        [](std::vector<std::any> args) -> std::any {
            int a = std::any_cast<int>(args[0]);
            return a + 32;
        }
    };

    cluster.submit(t1);
    cluster.submit(t2);

    // In v0.2 we assumed locations at dispatch time;
    // real completion tracking comes next (heartbeats/object reports).
    // For now you can just sleep or block at node-local store if you expose it.

    std::cout << "Cluster scheduled tasks.\n";

    n1.stop();
    n2.stop();
}