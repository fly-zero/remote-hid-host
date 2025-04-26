#pragma once

#include <WinSock2.h>

#include <boost/coroutine2/coroutine.hpp>
#include <boost/intrusive/list.hpp>

#include "connection.h"
#include "event_dispatch.h"

namespace remote_hid
{

class server final : public io_listener
{
    struct init;

    using co_push         = boost::coroutines2::coroutine<void>::pull_type;
    using co_pull         = boost::coroutines2::coroutine<void>::push_type;
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

    void run(co_pull &source);

    void yield() const;

    void resume();

private:
    event_dispatch &dispatcher_;                                       ///< event dispatcher
    co_push         sink_{ [this](co_pull &source) {run(source); } };  ///< coroutine sink
    co_pull        *source_{ nullptr };                                ///< coroutine source
    connection_list conn_list_{};                                      ///< connection list
};

inline void server::yield() const
{
    (*source_)();
}

inline void server::resume()
{
    sink_();
}

}
