#pragma once

#include <WinSock2.h>

#include <cassert>
#include <cstdint>

namespace remote_hid
{

class io_listener
{
    friend class event_dispatch;

    enum class handle_type : uint8_t { null, handle, socket };

public:
    explicit io_listener(HANDLE handle);

    explicit io_listener(SOCKET sock);

    virtual ~io_listener();

    io_listener(const io_listener &)    = delete;
    void operator=(const io_listener &) = delete;
    io_listener(io_listener &&)         = delete;
    void operator=(io_listener &&)      = delete;

    HANDLE get_handle() const;

    SOCKET get_socket() const;

protected:
    virtual void on_io_complete(DWORD bytes_transferred) = 0;

private:
    handle_type type_{handle_type::null};
    uintptr_t   handle_{~0UL};
};

inline io_listener::io_listener(HANDLE const handle)
    : type_{handle_type::handle}, handle_{reinterpret_cast<uintptr_t>(handle)}
{
}

inline io_listener::io_listener(SOCKET const sock) : type_{handle_type::socket}, handle_{sock} {}

inline HANDLE io_listener::get_handle() const
{
    assert(type_ == handle_type::handle);
    return reinterpret_cast<HANDLE>(handle_);  // NOLINT(performance-no-int-to-ptr)
}

inline SOCKET io_listener::get_socket() const
{
    assert(type_ == handle_type::socket);
    return handle_;
}

}  // namespace remote_hid