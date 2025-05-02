#include "io_listener.h"

namespace remote_hid
{

io_listener::~io_listener()
{
    switch (type_)
    {
    case handle_type::null:
        break;
    case handle_type::handle:
        if (get_handle() != INVALID_HANDLE_VALUE)
        {
            CloseHandle(get_handle());
        }
        break;
    case handle_type::socket:
        if (handle_ != INVALID_SOCKET)
        {
            closesocket(get_socket());
        }
        break;
    }
}

}  // namespace remote_hid