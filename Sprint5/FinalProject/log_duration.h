#pragma once

#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)

class LogDuration
{
public:
    // заменим имя типа std::chrono::steady_clock
    // с помощью using для удобства
    using Clock = std::chrono::steady_clock;

    LogDuration(const std::string &id, std::ostream &output_stream = std::cerr)
        : id_(id), output_stream_(output_stream)
    {
    }

    ~LogDuration()
    {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        output_stream_ << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

private:
    const std::string id_;
    std::ostream &output_stream_;
    const Clock::time_point start_time_ = Clock::now();
};