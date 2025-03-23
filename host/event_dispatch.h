#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <chrono>
#include <queue>
#include <utility>

#include <boost/intrusive/list.hpp>

namespace remote_hid
{

class io_listener
{
    friend class event_dispatch;

public:
    io_listener(const io_listener&) = delete;
    void operator=(const io_listener&) = delete;
    io_listener(io_listener&&) = delete;
    void operator=(io_listener&&) = delete;

protected:
    virtual ~io_listener() = default;
    virtual void on_io_complete(DWORD bytes_transferred, ULONG_PTR key) = 0;

private:
    HANDLE handle_{ INVALID_HANDLE_VALUE };
};

class loop_listener
{
    friend class event_dispatch;

    using list_hook_t = boost::intrusive::list_member_hook<>;

public:
    loop_listener() = default;
    virtual ~loop_listener() = default;
    loop_listener(const loop_listener&) = delete;
    void operator=(const loop_listener&) = delete;
    loop_listener(loop_listener&&) = delete;
    void operator=(loop_listener&&) = delete;

protected:
    virtual void on_loop() = 0;

private:
    list_hook_t list_hook_{};
};

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

class event_dispatch
{
protected:
    using time_point_t = timeout_listener::time_point_t;

    struct timeout_listener_entry
    {
        timeout_listener_entry(timeout_listener* listener, time_point_t now);

        bool operator>(const timeout_listener_entry& other) const;

        timeout_listener *listener_{ nullptr };
        time_point_t      deadline_{};
    };

    using loop_listener_list_t = boost::intrusive::list
                                    < loop_listener
                                    , boost::intrusive::member_hook
                                        < loop_listener
                                        , loop_listener::list_hook_t
                                        , &loop_listener::list_hook_
                                        >
                                    , boost::intrusive::constant_time_size<false>
                                    >;

    using timeout_listener_queue_t = std::priority_queue
                                        < timeout_listener_entry
                                        , std::vector<timeout_listener_entry>
                                        , std::greater<>
                                        >;

public:
    explicit event_dispatch(unsigned long thread_num = 0);

    ~event_dispatch();

    event_dispatch(event_dispatch&& other) noexcept;

    event_dispatch& operator=(event_dispatch&& other) noexcept;

    event_dispatch(const event_dispatch&) = delete;

    void operator=(const event_dispatch&) = delete;

    bool register_listener(io_listener &listener) const;

    void register_listener(loop_listener& listener);

    void register_listener(timeout_listener& listener);

    void run();

protected:
    void on_loop();

    void on_timeout(time_point_t now);

private:
    bool                     running_{ false };              ///< indicates if the event dispatch is running
    HANDLE                   iocp_{ INVALID_HANDLE_VALUE };  ///< I/O completion port handle
    loop_listener_list_t     loop_listener_list_{};          ///< list of loop listeners
    timeout_listener_queue_t timeout_listeners_queue_{};     ///< priority queue of timeout listeners
};

inline event_dispatch::timeout_listener_entry::timeout_listener_entry(
    timeout_listener* listener, time_point_t const now)
        : listener_(listener)
        , deadline_(now + listener_->interval_)
{
}

inline bool event_dispatch::timeout_listener_entry::operator>(
    const timeout_listener_entry& other) const
{
    return deadline_ > other.deadline_;
}

inline event_dispatch::event_dispatch(event_dispatch&& other) noexcept
    : iocp_(std::exchange(other.iocp_, INVALID_HANDLE_VALUE))
{
}

inline bool event_dispatch::register_listener(io_listener& listener) const
{
    auto const ret = CreateIoCompletionPort(listener.handle_, iocp_, reinterpret_cast<ULONG_PTR>(&listener), 0);
    return ret == iocp_;
}

inline void event_dispatch::register_listener(loop_listener& listener)
{
    loop_listener_list_.push_back(listener);
}

inline void event_dispatch::register_listener(timeout_listener& listener)
{
    timeout_listeners_queue_.emplace(&listener, std::chrono::steady_clock::now());
}

}

