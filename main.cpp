#include <print>
#include <chrono>
#include "Clock.hpp"

int main() {
    Clock clock;

    std::print("Hello, World!\n");
    std::print("Current time: {}\n", clock.getCurrentTimeString());
    std::print("Milliseconds since epoch: {}\n", clock.getTime<std::chrono::milliseconds>());
    std::print("Seconds since epoch: {}\n", clock.getTime<std::chrono::seconds>());

    return 0;
}
