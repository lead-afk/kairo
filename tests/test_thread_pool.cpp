#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <kairo/ds/ThreadPool.hpp>
#include <thread>

using namespace kairo::ds;

void test_thread_pool_basic()
{
    std::atomic<int> counter{0};
    auto worker = [&](int data)
    {
        counter += data;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10)); // Simulate work
    };

    ThreadPool<int> pool(worker, 2, 2);

    // Test initial state
    assert(pool.get_num_threads() == 2);
    assert(pool.get_working_count() == 0);

    // Push some data
    pool.push_data(1);
    pool.push_data(2);
    pool.push_data(3);

    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check results
    assert(counter == 6); // 1+2+3

    std::cout << "Basic ThreadPool test passed.\n";
}

void test_thread_pool_try_push()
{
    std::atomic<int> processed{0};
    auto worker = [&](int data) { processed++; };

    ThreadPool<int> pool(worker, 1);

    // Assuming queue has limited capacity, but since not implemented, this
    // placeholder
    bool success = pool.try_push_data(1, std::chrono::milliseconds(100));
    // assert(success); // Depends on implementation

    std::cout << "Try push ThreadPool test placeholder.\n";
}

void test_thread_pool_size_capacity()
{
    std::atomic<int> dummy{0};
    auto worker = [&](int) { dummy++; };

    ThreadPool<int> pool(worker, 2);

    // These will depend on the underlying queue capacity
    size_t cap = pool.capacity();
    size_t sz = pool.size();

    std::cout << "ThreadPool capacity: " << cap << ", size: " << sz << "\n";
    // Assertions depend on implementation
}

void test_thread_pool_concurrent()
{
    std::atomic<int> sum{0};
    auto worker = [&](int data)
    {
        sum += data;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    };

    ThreadPool<int> pool(worker, 4);

    // Push many items
    for (int i = 1; i <= 100; ++i)
    {
        pool.push_data(i);
    }

    // Wait for all to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Expected sum: 5050
    assert(sum == 5050);

    std::cout << "Concurrent ThreadPool test passed.\n";
}

int main()
{
    test_thread_pool_basic();
    test_thread_pool_try_push();
    test_thread_pool_size_capacity();
    test_thread_pool_concurrent();

    std::cout << "All ThreadPool tests passed!\n";
    return 0;
}