//
// Created by Samarth Mahendra on 12/20/25.
//


// PURPOSE:
// --------
// Implements the Worker execution logic.
//
// WHAT YOU NEED TO IMPLEMENT:
// ----------------------------
// 1. Logic for safely enqueuing tasks.
// 2. Logic for safely dequeuing tasks.
// 3. Logic for executing the task's callable.
// 4. Returning or recording the task result.
//
// THINK ABOUT:
// ------------
// - What happens if the queue is empty?
// - Should execution block or return immediately?
// - Where does the result go? (for now, return it directly)
//
// NOTE:
// -----
// This file should contain ZERO scheduling logic.
// It only executes what it's given.
#include "worker.h"
#include "object_store.h"
#include <functional>
#include <any>
#include <optional>

namespace orion {
    Worker::Worker(ObjectStore& store)
    : store_(store) {}


        ObjectRef Worker::submit(Task task) {
            ObjectRef ref{task.id};
            {
              // using lock guard for automatic mutex management instead of manual lock/unlock to avoid deadlocks
              std::lock_guard<std::mutex> lock(tasks_mutex);
              // using move to avoid copying the task, don't change

              task_queue.push({std::move(task), ref});
            }
            cv.notify_one(); // Notify one waiting thread that a new task is available
            return ref;
        }

    void Worker::run() {
        // Dequeue a task
        std::pair<Task, ObjectRef> item = [&] {
            std::unique_lock<std::mutex> lock(tasks_mutex);
            if (task_queue.empty()) {
                throw std::runtime_error("run() called with empty queue");
            }
            auto v = std::move(task_queue.front());
            task_queue.pop();
            return v;
        }();

        // Resolve dependency values (blocking is OK here)
        std::vector<std::any> args;
        args.reserve(item.first.deps.size());

        for (const auto& ref : item.first.deps) {
            args.push_back(store_.get_blocking(ref.id));
        }

        // Execute task with resolved dependency values
        std::any result = item.first.work(std::move(args));

        // Publish result to object store
        store_.put(item.second.id, std::move(result));
    }

        std::optional<Task> Worker::peek() {
            std::lock_guard<std::mutex> lock(tasks_mutex);
            if (task_queue.empty()) {
                return std::nullopt;
            }
            return task_queue.front().first;
        }
}
