#include "Clock.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>

Clock::Clock() : startTime(std::chrono::system_clock::now()) {}

std::string Clock::getCurrentTimeString() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Clock::reset() {
    startTime = std::chrono::system_clock::now();
}

long long Clock::getElapsedMilliseconds() const {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return elapsed.count();
}

long long Clock::getElapsedSeconds() const {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
    return elapsed.count();
}
