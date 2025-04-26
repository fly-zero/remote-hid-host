#pragma once

#include <WinSock2.h>

#include <boost/intrusive/list.hpp>

#include "connection.h"
#include "coroutine.h"
#include "event_dispatch.h"

namespace remote_hid
{

class server final : public io_listener, public loop_listener, public coroutine_base
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

    event_dispatch &get_event_dispatch() const;

    void close(connection &conn);

protected:
    void on_io_complete(DWORD bytes_transferred) override;

    void on_loop() override;

    static SOCKET listen(std::string_view addr);

    [[nodiscard]] SOCKET accept() const;

    void run() override;

private:
    event_dispatch &dispatcher_;     ///< event dispatcher
    connection_list active_list_{};  ///< active connection list
    connection_list closed_list_{};  ///< closed connection list
};

inline event_dispatch &server::get_event_dispatch() const { return dispatcher_; }

inline void server::close(connection &conn)
{
    closed_list_.splice(closed_list_.end(), active_list_, active_list_.iterator_to(conn));
}

}  // namespace remote_hid