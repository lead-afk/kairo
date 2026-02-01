#pragma once

namespace kairo::ds
{
/**
 * @brief RAII helper that increments a counter on construction and
 * decrements it on destruction.
 */
template <typename T>
struct CounterGuard
{
    T &counter;
    CounterGuard(T &c) : counter(c) { counter = counter + 1; }
    ~CounterGuard() { counter = counter - 1; }
};
} // namespace kairo::ds