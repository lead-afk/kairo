#pragma once
#include "kairo/ds/CounterGuard.hpp"
#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <kairo/ds/ThreadSafeQueue.hpp>
#include <thread>

namespace kairo::ds
{
/**
 * @brief Thread pool that processes tasks using worker threads.
 *
 * Each worker repeatedly pops items from the internal queue and invokes the
 * provided worker function. The pool can optionally swallow exceptions or log
 * them without terminating.
 *
 * @tparam T Type of data elements to process.
 */
template <typename T>
class ThreadPool
{
  private:
    ThreadSafeQueue<T> data_queue;
    std::vector<std::thread> workers;
    std::atomic_bool threads_alive{false};
    const std::function<void(T)> worker_function;
    std::atomic<size_t> working_count{0};
    const bool swallow_exceptions;
    const bool mute_exceptions;

    /**
     * @brief Main loop executed by each worker thread.
     */
    void worker_thread_loop();

  public:
    /**
     * @brief Constructs thread pool with specified worker function.
     * @param worker_function Function to execute for each data item.
     * @param queue_size Maximum size of the work queue.
     * @param num_threads Number of worker threads (defaults to hardware
     * concurrency).
     * @param swallow_exceptions If true, exceptions in worker threads are
     * swallowed instead of terminating the process.
     * @param mute_exceptions If true, suppresses exception logging.
     */
    ThreadPool(std::function<void(T)> worker_function, size_t queue_size = 1024,
               size_t num_threads = std::thread::hardware_concurrency(),
               bool swallow_exceptions = false, bool mute_exceptions = false);

    /**
     * @brief Destructor stops all workers and joins threads.
     */
    ~ThreadPool();

    /**
     * @brief Blocks until space is available, then enqueues data for
     * processing.
     * @param data Element to add to the work queue.
     */
    template <typename U>
    void push_data(U &&data);

    /**
     * @brief Attempts to enqueue data with timeout.
     * @param data Element to add to the work queue.
     * @param timeout Maximum time to wait for queue space.
     * @return True if data was enqueued, false if timeout occurred.
     */
    template <typename U, typename Duration>
    bool try_push_data(U &&data, const Duration &timeout);

    /**
     * @brief Returns the number of worker threads.
     * @return Number of active worker threads in the pool.
     */
    size_t get_num_threads() const;

    /**
     * @brief Returns the current queue size.
     * @return Size of the underlying work queue.
     */
    size_t size() const;

    /**
     * @brief Returns the maximum queue capacity.
     * @return Capacity of the underlying work queue.
     */
    size_t capacity() const;

    /**
     * @brief Returns the number of items currently being processed.
     * @return Number of items currently being processed by workers.
     */
    size_t get_working_count() const;
};

template <typename T>
ThreadPool<T>::ThreadPool(std::function<void(T)> worker_function,
                          size_t queue_size, size_t num_threads,
                          bool swallow_exceptions, bool mute_exceptions)
    : data_queue(queue_size), worker_function(worker_function),
      swallow_exceptions(swallow_exceptions), mute_exceptions(mute_exceptions)
{
    threads_alive = true;
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers.emplace_back(&ThreadPool<T>::worker_thread_loop, this);
    }
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
    // std::cout << "Shutting down ThreadPool with " << workers.size()
    //           << " threads.\n";
    threads_alive = false;
    data_queue.close();
    for (auto &worker : workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

template <typename T>
void ThreadPool<T>::worker_thread_loop()
{
    while (threads_alive)
    {
        auto item = std::move(data_queue.pop());
        if (item)
        {
            kairo::ds::CounterGuard guard(working_count);
            try
            {
                worker_function(*item);
            }
            catch (std::exception &e)
            {
                if (!swallow_exceptions)
                {
                    std::cerr << "ThreadPool worker exception: " << e.what()
                              << "\n";
                    threads_alive = false;
                    data_queue.close();
                    std::terminate();
                }
                if (!mute_exceptions)
                {
                    std::cerr
                        << "ThreadPool worker swallowed exception: " << e.what()
                        << "\n";
                }
            }
            catch (...)
            {
            }
        }
    }
    // std::cout << "ThreadPool worker thread exiting.\n";
}

template <typename T>
template <typename U>
void ThreadPool<T>::push_data(U &&data)
{
    data_queue.push(std::forward<U>(data));
}

template <typename T>
template <typename U, typename Duration>
bool ThreadPool<T>::try_push_data(U &&data, const Duration &timeout)
{
    return data_queue.try_push(std::forward<U>(data), timeout);
}

template <typename T>
size_t ThreadPool<T>::get_num_threads() const
{
    return workers.size();
}

template <typename T>
size_t ThreadPool<T>::size() const
{
    return data_queue.size();
}

template <typename T>
size_t ThreadPool<T>::capacity() const
{
    return data_queue.capacity();
}

template <typename T>
size_t ThreadPool<T>::get_working_count() const
{
    return working_count.load();
}

} // namespace kairo::ds