#pragma once
#include "kairo/ds/ThreadPool.hpp"
#include "kairo/ds/ThreadSafeQueue.hpp"
#include <cstddef>
#include <functional>
#include <optional>
#include <utility>

namespace kairo::ds
{
template <typename T>
/**
 * @brief Buffered constructor that pre-constructs items of type T in the
 * background using a thread pool.
 *
 * This class maintains a buffer of pre-constructed items, allowing for
 * efficient retrieval without blocking on construction. A thread pool is used
 * to construct items in the background.
 *
 * @tparam T Type of items to construct and buffer.
 */
class BufferedConstructor
{
  private:
    ThreadSafeQueue<T> ready_items;
    ThreadPool<bool> thread_pool;

    const std::function<T()> constructor_function;

  public:
    BufferedConstructor(
        size_t max_buffer_size, std::function<T()> constructor_function,
        size_t num_threads = std::thread::hardware_concurrency(),
        bool prefill = false);

    ~BufferedConstructor();

    /**
     * @brief Retrieves a constructed item from the buffer, blocking if none
     * are available.
     * @return Constructed item of type T.
     */
    std::optional<T> get_item();

    /**
     * @brief Attempts to retrieve a constructed item with timeout.
     * @tparam Duration Type representing the timeout duration.
     * @param timeout Maximum time to wait for an item.
     * @return Optional containing the constructed item if available, or
     * std::nullopt if timed out.
     */
    template <typename Duration>
    std::optional<T> try_get_item(const Duration &timeout);

    /**
     * @brief Returns the current number of buffered items.
     * @return Number of items currently in the buffer.
     */
    size_t buffered_size() const { return ready_items.size(); }

    /**
     * @brief Returns the maximum buffer capacity.
     * @return Maximum number of items the buffer can hold.
     */
    size_t buffer_capacity() const { return ready_items.capacity(); }

    /**
     * @brief Returns the number of items currently being constructed.
     * @return Number of items currently under construction.
     */
    size_t constructing_count() const
    {
        return thread_pool.get_working_count();
    }
};

template <typename T>
BufferedConstructor<T>::BufferedConstructor(
    size_t max_buffer_size, std::function<T()> constructor_function,
    size_t num_threads, bool prefill)
    : ready_items(max_buffer_size), constructor_function(constructor_function),
      thread_pool(
          [this](bool)
          {
              T item = std::forward<T>(this->constructor_function)();
              ready_items.push(std::move(item));
          },
          2 * max_buffer_size, num_threads, true, true)
{
    if (prefill)
    {
        for (size_t i = 0; i < max_buffer_size; ++i)
        {
            thread_pool.push_data(true);
        }
    }
}

template <typename T>
BufferedConstructor<T>::~BufferedConstructor()
{
    ready_items.close();
}

template <typename T>
std::optional<T> BufferedConstructor<T>::get_item()
{
    if (ready_items.size() < ready_items.capacity() / 2)
    {
        thread_pool.push_data(true);
    }
    thread_pool.push_data(true);

    return ready_items.pop();
}

template <typename T>
template <typename Duration>
std::optional<T> BufferedConstructor<T>::try_get_item(const Duration &timeout)
{
    if (ready_items.size() < ready_items.capacity() / 2)
    {
        thread_pool.try_push_data(true, timeout);
    }
    thread_pool.try_push_data(true, timeout);

    return ready_items.try_pop(timeout);
}
} // namespace kairo::ds