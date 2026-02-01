#pragma once
#include "kairo/ds/CounterGuard.hpp"
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace kairo::ds
{

/**
 * @brief Thread-safe bounded queue using condition variables and mutexes for
 * synchronization.
 *
 * Provides blocking and timed push/pop operations. Closing the queue unblocks
 * waiting threads and resets the queue state.
 *
 * @tparam T Type of elements stored in the queue.
 */
template <typename T>
class ThreadSafeQueue
{
  private:
    size_t head = 0;
    size_t tail = 0;
    size_t max_size;
    size_t current_size = 0;
    std::atomic_size_t currently_pushing = 0;
    std::atomic_size_t currently_popping = 0;
    mutable std::mutex mutex;

    std::condition_variable not_empty;
    std::condition_variable not_full;

    std::atomic<bool> closed = false;

    std::vector<T> buffer;

  public:
    /**
     * @brief Constructs a queue with fixed capacity.
     * @param size Maximum number of elements the queue can hold.
     */
    ThreadSafeQueue(size_t size);

    ~ThreadSafeQueue() = default;

    /**
     * @brief Blocks until space is available, then pushes item to queue.
     * @param item Element to add to the queue.
     * @return True if the item was pushed, false if the queue was closed.
     */
    template <typename U>
    bool push(U &&item);

    /**
     * @brief Attempts to push item with timeout.
     * @param item Element to add to the queue.
     * @param timeout Maximum time to wait for space.
     * @return True if item was pushed, false if timeout occurred.
     */

    template <typename U, typename Duration>
    bool try_push(U &&item, const Duration &timeout);

    /**
     * @brief Attempts to push item with timeout.
     * @param item Element to add to the queue.
     * @param timeout Maximum time to wait for space.
     * @return True if item was pushed, false if timeout occurred.
     */
    template <typename Duration>
    bool try_push(T &&item, const Duration &timeout);

    /**
     * @brief Blocks until item is available, then removes and returns it.
     * @return Element from the front of the queue, or std::nullopt if the queue
     * is closed.
     */
    std::optional<T> pop();

    /**
     * @brief Attempts to pop item with timeout.
     * @param timeout Maximum time to wait for an item.
     * @return Element if available, empty optional if timeout occurred.
     */
    template <typename Duration>
    std::optional<T> try_pop(const Duration &timeout);

    /**
     * @brief Resets the queue to an empty state, unblocking all waiting
     * threads. Waiting threads will receive empty results. Queue is reopened
     * after clearing. While clearing, no other operations can be performed on
     * the queue. By default, only unblocks waiting threads without removing
     * items. They will be removed as push operations overwrite them.
     * @param full_clear If true, removes all items from the queue.
     * @return void
     */
    void clear(bool full_clear = false);
    // {
    //     closed.store(true);
    //     items.release(current_waiting.load());
    //     available_slots.release(current_size.load());
    // }

    /**
     * @brief Returns the current number of elements in the queue.
     * @return Number of elements in the queue.
     */
    size_t size() const;

    /**
     * @brief Returns the maximum capacity of the queue.
     * @return Maximum number of elements the queue can hold.
     */
    size_t capacity() const;

    size_t pushing_count() const { return currently_pushing; }
    size_t popping_count() const { return currently_popping; }
};

template <typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(size_t size) : max_size(size), buffer(size)
{
}

template <typename T>
template <typename U>
bool ThreadSafeQueue<T>::push(U &&item)
{
    std::unique_lock<std::mutex> lock(mutex);
    CounterGuard guard(currently_pushing);
    not_full.wait(lock, [this]()
                  { return closed.load() || current_size < max_size; });

    if (closed.load() || current_size >= max_size)
        return false;

    buffer[tail] = std::forward<U>(item);
    tail = (tail + 1) % max_size;
    ++current_size;

    not_empty.notify_one();
    return true;
}

template <typename T>
template <typename U, typename Duration>
bool ThreadSafeQueue<T>::try_push(U &&item, const Duration &timeout)
{
    std::unique_lock<std::mutex> lock(mutex);
    CounterGuard guard(currently_pushing);
    if (!not_full.wait_for(
            lock, timeout,
            [this]() { return closed.load() || current_size < max_size; }))
    {
        return false;
    }
    if (closed.load() || current_size >= max_size)
    {
        return false;
    }
    buffer[tail] = std::forward<U>(item);

    tail = (tail + 1) % max_size;
    ++current_size;
    not_empty.notify_one();
    return true;
}

template <typename T>
std::optional<T> ThreadSafeQueue<T>::pop()
{
    std::unique_lock<std::mutex> lock(mutex);
    CounterGuard guard(currently_popping);
    not_empty.wait(lock,
                   [this]() { return closed.load() || current_size > 0; });
    if (closed.load() || current_size == 0)
    {
        return std::nullopt;
    }

    T item = std::move(buffer[head]);
    head = (head + 1) % max_size;
    --current_size;
    not_full.notify_one();
    return item;
}

template <typename T>
template <typename Duration>
std::optional<T> ThreadSafeQueue<T>::try_pop(const Duration &timeout)
{
    std::unique_lock<std::mutex> lock(mutex);
    CounterGuard guard(currently_popping);
    if (!not_empty.wait_for(lock, timeout, [this]()
                            { return closed.load() || current_size > 0; }))
    {
        return std::nullopt;
    }
    if (closed.load() || current_size == 0)
    {
        return std::nullopt;
    }

    T item = std::move(buffer[head]);
    head = (head + 1) % max_size;
    --current_size;
    not_full.notify_one();
    return item;
}

template <typename T>
void ThreadSafeQueue<T>::clear(bool full_clear)
{
    std::unique_lock<std::mutex> lock(mutex);
    closed.store(true);
    not_empty.notify_all();
    not_full.notify_all();

    head = 0;
    tail = 0;
    current_size = 0;

    if (full_clear)
    {
        buffer.clear();
        buffer.resize(max_size);
    }
    closed.store(false);
}

template <typename T>
size_t ThreadSafeQueue<T>::size() const
{
    std::unique_lock<std::mutex> lock(mutex);
    return current_size;
}

template <typename T>
size_t ThreadSafeQueue<T>::capacity() const
{
    return max_size;
}

} // namespace kairo::ds
