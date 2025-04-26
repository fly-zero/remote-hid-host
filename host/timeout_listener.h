#pragma once

#include <chrono>

namespace remote_hid
{

class timeout_listener
{
    friend class event_dispatch;

    using duration_t = std::chrono::steady_clock::duration;
    using time_point_t = std::chrono::steady_clock::time_point;

public:
    timeout_listener() = default;

    virtual ~timeout_listener() = default;

    timeout_listener(const timeout_listener&) = delete;
    void operator=(const timeout_listener&) = delete;
    timeout_listener(timeout_listener&&) = delete;
    void operator=(timeout_listener&&) = delete;

protected:
    virtual void on_timeout(time_point_t now) = 0;

private:
    duration_t interval_{};  ///< timeout interval
};

}
