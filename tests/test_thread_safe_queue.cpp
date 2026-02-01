#include <cassert>
#include <chrono>
#include <iostream>
#include <kairo/ds/ThreadSafeQueue.hpp>
#include <thread>

using namespace kairo::ds;

void test_thread_safe_queue_basic()
{
    ThreadSafeQueue<int> queue(5);

    // Test capacity
    assert(queue.capacity() == 5);
    assert(queue.size() == 0);

    // Test push and pop
    queue.push(1);
    assert(queue.size() == 1);

    auto result = queue.pop();
    assert(result.has_value());
    assert(*result == 1);
    assert(queue.size() == 0);

    std::cout << "Basic ThreadSafeQueue tests passed.\n";
}

void test_thread_safe_queue_try_push_pop()
{
    ThreadSafeQueue<int> queue(2);

    // Test try_push
    bool success = queue.try_push(1, std::chrono::milliseconds(100));
    assert(success);
    assert(queue.size() == 1);

    success = queue.try_push(2, std::chrono::milliseconds(100));
    assert(success);
    assert(queue.size() == 2);

    // Queue should be full now
    success = queue.try_push(3, std::chrono::milliseconds(10));
    assert(!success); // Should fail quickly

    // Test try_pop
    auto result = queue.try_pop(std::chrono::milliseconds(100));
    assert(result.has_value());
    assert(*result == 1);

    result = queue.try_pop(std::chrono::milliseconds(100));
    assert(result.has_value());
    assert(*result == 2);

    // Queue should be empty
    result = queue.try_pop(std::chrono::milliseconds(10));
    assert(!result.has_value());

    std::cout << "Try push/pop ThreadSafeQueue tests passed.\n";
}

void test_thread_safe_queue_close()
{
    ThreadSafeQueue<int> queue(5);
    queue.push(1);
    queue.push(2);

    queue.clear();

    // After close, pop should return nullopt
    auto result = queue.pop();
    assert(!result.has_value());

    result = queue.try_pop(std::chrono::milliseconds(10));
    assert(!result.has_value());

    std::cout << "Close ThreadSafeQueue test passed.\n";
}

void test_thread_safe_queue_concurrent()
{
    ThreadSafeQueue<int> queue(10);

    std::thread producer(
        [&]()
        {
            for (int i = 0; i < 100; ++i)
            {
                queue.push(i);
            }
        });

    std::thread consumer(
        [&]()
        {
            int sum = 0;
            for (int i = 0; i < 100; ++i)
            {
                auto result = queue.pop();
                if (result)
                {
                    sum += *result;
                }
            }
            // Expected sum: 0+1+...+99 = 4950
            assert(sum == 4950);
        });

    producer.join();
    consumer.join();

    std::cout << "Concurrent ThreadSafeQueue test passed.\n";
}

int main()
{
    test_thread_safe_queue_basic();
    test_thread_safe_queue_try_push_pop();
    test_thread_safe_queue_close();
    test_thread_safe_queue_concurrent();

    std::cout << "All ThreadSafeQueue tests passed!\n";
    return 0;
}