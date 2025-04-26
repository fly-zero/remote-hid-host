#include "server.h"

#include <WS2tcpip.h>
#include <mswsock.h>

#include <memory>
#include <string>
#include <system_error>

#include "common.h"
#include "connection.h"

namespace remote_hid
{

inline LPFN_ACCEPTEX s_accept_ex = nullptr;

inline LPFN_ACCEPTEX load_accept_ex(SOCKET sock)
{
    // Load the AcceptEx function into memory using WSAIoctl.
    // The WSAIoctl function is an extension of the ioctlsocket()
    // function that can use overlapped I/O. The function's 3rd
    // through 6th parameters are input and output buffers where
    // we pass the pointer to our AcceptEx function. This is used
    // so that we can call the AcceptEx function directly, rather
    // than refer to the Mswsock.lib library.

    GUID guid_accept_ex = WSAID_ACCEPTEX;
    LPFN_ACCEPTEX accept_ex = nullptr;
    DWORD bytes;

    auto const res = WSAIoctl(
        sock,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guid_accept_ex,
        sizeof(guid_accept_ex),
        &accept_ex,
        sizeof(accept_ex),
        &bytes,
        nullptr,
        nullptr);
    if (res == SOCKET_ERROR) {
        auto const err = WSAGetLastError();
        closesocket(sock);
        throw std::system_error(err, std::system_category(), "WSAIoctl failed");
    }

    return accept_ex;
}

struct server::init
{
    init();

    ~init();

    init(const init&) = delete;
    void operator=(const init&) = delete;
    init(init&&) = delete;
    void operator=(init&&) = delete;

    static init s_init;
};

server::init server::init::s_init;

std::tuple<std::string_view, std::string_view> split_string(std::string_view str, char delim)
{
    auto const pos = str.find(delim);
    if (pos == std::string_view::npos)
    {
        return { str, {} };
    }

    return { str.substr(0, pos), str.substr(pos + 1) };
}

server::server(event_dispatch& dispatcher, std::string_view addr)
    : io_listener{ listen(addr) }
    , dispatcher_{ dispatcher }
{
}

void server::on_io_complete(DWORD bytes_transferred)
{
    resume();
}

inline SOCKET server::listen(std::string_view addr)
{
    // Create a socket
    auto const sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "Failed to create socket");
    }

    // spit address
    auto const [host, port] = split_string(addr, ':');
    if (host.empty() || port.empty())
    {
        closesocket(sock);
        throw exception::invalid_argument("Invalid address: '%.*s'", static_cast<int>(addr.size()), addr.data());
    }

    // copy host to null-terminated string
    auto const host_deleter = [](char* p) { _freea(p); };
    std::unique_ptr<char, decltype(host_deleter)> const host_str{ static_cast<char*>(_malloca(host.size() + 1)), host_deleter };
    *std::copy(host.begin(), host.end(), host_str.get()) = '\0';

    // setup address
    sockaddr_in tmp{};
    tmp.sin_family = AF_INET;
    tmp.sin_port = htons(static_cast<uint16_t>(std::stoi(port.data())));
    if (inet_pton(AF_INET, host_str.get(), &tmp.sin_addr) != 1)
    {
        auto const err = WSAGetLastError();
        closesocket(sock);
        throw std::system_error(err, std::system_category(), "Invalid address");
    }

    // bind socket
    if (bind(sock, reinterpret_cast<sockaddr*>(&tmp), sizeof(tmp)) == SOCKET_ERROR)
    {
        auto const err = WSAGetLastError();
        closesocket(sock);
        throw std::system_error(err, std::system_category(), "bind failed");
    }

    // listen
    if (::listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        auto const err = WSAGetLastError();
        closesocket(sock);
        throw std::system_error(err, std::system_category(), "listen failed");
    }

    // setup member
    return sock;
}

SOCKET server::accept() const
{
    // create a client socket
    auto const client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock == INVALID_SOCKET)
    {
        throw std::system_error(WSAGetLastError(), std::system_category(), "socket");
    }

    // accept a connection
    char buff[1024];
    DWORD bytes = 0;
    WSAOVERLAPPED listen_overlapped{};
    if (!s_accept_ex(get_socket(),
                     client_sock,
                     buff,
                     sizeof buff - (sizeof(sockaddr_in) + 16) * 2,
                     sizeof(sockaddr_in) + 16,
                     sizeof(sockaddr_in) + 16,
                     &bytes,
                     &listen_overlapped))
    {
        if (WSAGetLastError() != ERROR_IO_PENDING)
        {
            throw std::system_error(WSAGetLastError(), std::system_category(), "AcceptEx");
        }
    }

    // switch to main coroutine
    yield();

    // get the client socket
    return client_sock;
}

void server::run()
{
    // get the AcceptEx function
    s_accept_ex = load_accept_ex(get_socket());
    assert(s_accept_ex);

    // register the socket to the dispatcher
    dispatcher_.register_listener(*this);

    do
    {
        auto const client_sock = accept();
        auto const conn = new connection{ client_sock };
        conn_list_.push_back(*conn);

        // TODO: register client_sock to event_dispatch
    } while (true);
}

inline server::init::init()
{
    // Initialize Winsock
    WSADATA wsa_data;
    if (auto const err = WSAStartup(MAKEWORD(2, 2), &wsa_data); err != 0)
    {
        throw std::system_error(err, std::system_category(), "WSAStartup failed");
    }
}

inline server::init::~init()
{
    WSACleanup();
}

}
