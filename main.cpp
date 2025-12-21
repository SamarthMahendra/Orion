//
// Created by Samarth Mahendra on 12/21/25.
//

#include "worker.h"
#include "task.h"
#include "object_store.h"

#include <iostream>
#include <any>        // ✅ REQUIRED for std::any_cast
#include <string>     // ✅ fixed typo


int main(){
    orion::ObjectStore store;
    orion::Worker worker(store);


    // simple task
    orion::Task task("task1", [] {
    return 42;          // int
    });

    orion::ObjectRef ref = worker.submit(task);

    // enqueue the task to the worker
    worker.run();
    auto result = store.get(ref.id);
    if (result) {
        std::cout << "Result: "
                  << std::any_cast<int>(*result)
                  << std::endl;
    }

    return 0;

}