#pragma once

#include<thread>
#include<iostream>
#include<functional>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<memory>
#include<utility>
#include<tuple>

namespace my_pool{
    class thread_pool{
        public:
            explicit thread_pool(std::size_t thread_count = std::thread::hardware_concurrency());

            using work_item_t = std::tuple<std::function<void(int)>,int>;

            thread_pool(const thread_pool&) = delete;
            thread_pool(thread_pool&&) = delete;

            thread_pool& operator=(const thread_pool&) = delete;
            thread_pool& operator=(thread_pool&&) = delete;

            /**
             * This function acts like a producer
            */
            void do_task(work_item_t task);
            ~thread_pool();
        private:

            /**
                * 1. We need to have a task queue which will be containing the task that need to be executed by the threads
                * 2. We need to have a vector containing all the threads_ids
            */
            using work_item_ptr_t = std::unique_ptr<work_item_t>;
            using work_queue_t = std::queue<work_item_ptr_t>;

            work_queue_t task_queue;

            std::mutex m_queue_lock;
            std::condition_variable m_condition;

            using threads_t = std::vector<std::thread>;
            threads_t m_threads;
    };

}