#pragma once

#include <WinSock2.h>

#include <boost/intrusive/list_hook.hpp>

#include "coroutine.h"
#include "event_dispatch.h"

namespace remote_hid
{

class connection final : public io_listener, public coroutine_base
{
    using list_hook = boost::intrusive::list_member_hook<>;

    friend class server;

public:
    connection(server &server, SOCKET sock);

    ~connection() override = default;

    connection(const connection &)     = delete;
    connection(connection &&other)     = delete;
    void operator=(const connection &) = delete;
    void operator=(connection &&other) = delete;

protected:
    void on_io_complete(DWORD bytes_transferred) override;

    void run() override;

    long recv(char *buffer, unsigned size) const;

private:
    list_hook list_hook_{};
    server   &server_;
};

inline connection::connection(server &server, SOCKET sock) : io_listener{sock}, server_{server}
{
    resume();
}

}  // namespace remote_hid