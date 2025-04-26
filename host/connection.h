#pragma once

#include <WinSock2.h>

#include <boost/intrusive/list_hook.hpp>

#include "event_dispatch.h"

namespace remote_hid
{

class connection final : public io_listener
{
    using list_hook = boost::intrusive::list_member_hook<>;

    friend class server;

public:
    explicit connection(SOCKET sock);

    ~connection() override = default;

    connection(const connection&) = delete;
    connection(connection&& other) = delete;
    void operator=(const connection&) = delete;
    void operator=(connection&& other) = delete;

protected:
    void on_io_complete(DWORD bytes_transferred) override;

private:
    list_hook list_hook_{};
};

inline connection::connection(SOCKET const sock)
    : io_listener{ sock }
{
}

}

