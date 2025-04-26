#pragma once

#include <WinSock2.h>

#include <boost/intrusive/list.hpp>

#include "connection.h"
#include "coroutine.h"
#include "event_dispatch.h"

namespace remote_hid
{

class server final : public io_listener, public coroutine_base
{
    struct init;

    using connection_list = boost::intrusive::list
                                < connection
                                , boost::intrusive::member_hook
                                    < connection
                                    , connection::list_hook
                                    , &connection::list_hook_
                                    >
                                >;

public:
    server(event_dispatch &dispatcher, std::string_view addr);

    ~server() override = default;

    server(server&& other) = delete;
    void operator=(server&& other) = delete;
    server(server const&) = delete;
    server& operator=(server const&) = delete;

protected:
    void on_io_complete(DWORD bytes_transferred) override;

    static SOCKET listen(std::string_view addr);

    [[nodiscard]] SOCKET accept() const;

    void run() override;

private:
    event_dispatch &dispatcher_;                                       ///< event dispatcher
    connection_list conn_list_{};                                      ///< connection list
};

}
