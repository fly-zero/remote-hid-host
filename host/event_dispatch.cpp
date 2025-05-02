#include "event_dispatch.h"

#include <system_error>

#include "common.h"

namespace remote_hid
{

event_dispatch::event_dispatch(unsigned long const thread_num)
    : iocp_{CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, thread_num)}
{
    if (!iocp_)
    {
        throw std::system_error(static_cast<int>(GetLastError()),
                                std::system_category(),
                                "Failed to create I/O completion port");
    }

    running_ = true;
}

event_dispatch::~event_dispatch()
{
    if (iocp_)
    {
        CloseHandle(iocp_);
    }
}

event_dispatch &event_dispatch::operator=(event_dispatch &&other) noexcept
{
    if (this != &other)
    {
        // close the current handle
        if (iocp_ != INVALID_HANDLE_VALUE)
        {
            CloseHandle(iocp_);
        }

        // move the handle
        iocp_ = std::exchange(other.iocp_, INVALID_HANDLE_VALUE);
    }

    return *this;
}

void event_dispatch::run()
{
    while (running_)
    {
        // call on_loop for all loop listeners
        on_loop();

        DWORD        bytes_transferred{};
        ULONG_PTR    key{};
        LPOVERLAPPED overlapped;
        if (auto const ret =
                GetQueuedCompletionStatus(iocp_, &bytes_transferred, &key, &overlapped, 50);
            ret)
        {
            auto const listener =
                reinterpret_cast<io_listener *>(key);  // NOLINT(performance-no-int-to-ptr)
            listener->on_io_complete(bytes_transferred);
        }
        else if (overlapped)
        {
            // TODO: handle error
        }
        else if (GetLastError() == WAIT_TIMEOUT)
        {
            on_timeout(std::chrono::steady_clock::now());
        }
        else
        {
            throw std::system_error(static_cast<int>(GetLastError()),
                                    std::system_category(),
                                    "Failed to get queued completion status");
        }
    }
}

inline void event_dispatch::on_loop()
{
    for (auto &listener : loop_listener_list_)
    {
        listener.on_loop();
    }
}

void event_dispatch::on_timeout(time_point_t const now)
{
    while (!timeout_listeners_queue_.empty())
    {
        // check if the top listener is not ready
        auto &top = timeout_listeners_queue_.top();
        if (now < top.deadline_)
        {
            break;
        }

        // call on_timeout for the listener
        auto const listener = top.listener_;
        listener->on_timeout(now);

        // remove the listener from the queue
        timeout_listeners_queue_.pop();

        // reinsert the listener with the new deadline
        timeout_listeners_queue_.emplace(listener, now);
    }
}

}  // namespace remote_hid
