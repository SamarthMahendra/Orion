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

#include "distributed/node_runtime.h"

int main() {
    orion::distributed::NodeRuntime node(4, 50051);

    node.start();

    auto& rt = node.local_runtime();

    orion::Task t{
        "A",
        {},
        [](std::vector<std::any>) { return 42; }
    };

    auto ref = rt.submit(t);
    rt.wait(ref);

    node.stop();
}