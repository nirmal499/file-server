#include <thread_pool/pool.hpp>

my_pool::thread_pool::thread_pool(std::size_t thread_count)
{
    if (!thread_count)
    {
        throw std::invalid_argument("bad thread count. It must be non zero");
    }

    m_threads.reserve(thread_count);

    for (auto i = 0; i < thread_count; ++i)
    {

        /**
         * This function acts like the consumer
         */
        auto function_to_execute = [this]()
        {
            while (true)
            {
                work_item_ptr_t task{nullptr};
                {
                    std::unique_lock<std::mutex> gaurd(m_queue_lock);
                    /**
                     * All threads have to wait here till task_queue is not empty
                     * if task_queue is empty then task_queue.empty will return true -> wait function gets false as a result threads have to wait
                     * if task_queue is not empty then task_queue.empty will return false -> wait function gets true as a result thread can move on
                     */
                    m_condition.wait(gaurd, [&]()
                                     { return !task_queue.empty(); });

                    // front() -> returns the first element of the queue
                    task = std::move(task_queue.front());

                    // pop() -> removes an element from the front of the queue
                    task_queue.pop();
                }

                if (!task)
                {
                    // Incase if we have not task e.g task == nullptr then break out of the while loop
                    break;
                }

                // Here task is a pointer
                auto my_func = std::get<0>(*task);
                auto first_argument = std::get<1>(*task);

                // here we actually execute our task
                my_func(first_argument);

            }
        };

        std::thread worker(function_to_execute);

        m_threads.emplace_back(std::move(worker));
    }
}

void my_pool::thread_pool::do_task(work_item_t task)
{
    auto task_item = std::make_unique<work_item_t>(std::move(task));
    {
        std::unique_lock<std::mutex> gaurd(m_queue_lock);
        // push() -> inserts an element at the back of the queue
        task_queue.push(std::move(task_item));
    }

    /**
     * If there are ten threads blocked on the condition variable,
     * for example, notify_one() will unblock only one thread
     * while notify_all() will unblock them all
     */
    m_condition.notify_one();
}

my_pool::thread_pool::~thread_pool()
{
    // Our sentinel value is nullptr   
    {
        std::unique_lock<std::mutex> gaurd(m_queue_lock);
        for(auto &t:m_threads){
            task_queue.push(work_item_ptr_t{nullptr});
        }
    }

    /**
     * if the thread pool was never given work to do, 
     * upon deconstruction of the pool, you need to notify all to 
     * be able to exit the threads or else it will hang on join forever.
    */
    m_condition.notify_all(); // notify the all the sleeping threads to wake up

    for (auto &t : m_threads)
    {
        t.join();
    }
}