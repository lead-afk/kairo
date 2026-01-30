#pragma once

#include <chrono>
#include <string>

class Clock {
public:
    Clock();

    template<typename Duration>
    long long getTime() const {
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::time_point_cast<Duration>(now);
        return duration.time_since_epoch().count();
    }

    std::string getCurrentTimeString() const;
    void reset();
    long long getElapsedMilliseconds() const;
    long long getElapsedSeconds() const;

private:
    std::chrono::system_clock::time_point startTime;
};
